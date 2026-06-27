package com.studyback.hospitalservicesystem.agent.orchestrator;

import com.studyback.hospitalservicesystem.agent.aggregator.ContextAggregator;
import com.studyback.hospitalservicesystem.agent.dto.ActionPlan;
import com.studyback.hospitalservicesystem.agent.dto.AgentContext;
import com.studyback.hospitalservicesystem.agent.dto.AgentResponse;
import com.studyback.hospitalservicesystem.agent.dto.PlannedAction;
import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import com.studyback.hospitalservicesystem.agent.generator.AnswerGeneratorService;
import com.studyback.hospitalservicesystem.agent.generator.AnswerGeneratorService.GenerationResult;
import com.studyback.hospitalservicesystem.agent.memory.ChatMemoryService;
import com.studyback.hospitalservicesystem.agent.memory.UserMemoryService;
import com.studyback.hospitalservicesystem.agent.planner.IntentPlannerService;
import com.studyback.hospitalservicesystem.agent.rag.EmbeddingRouterService;
import com.studyback.hospitalservicesystem.agent.rag.MultiKBRetrievalService;
import com.studyback.hospitalservicesystem.agent.tool.ReportTool;
import com.studyback.hospitalservicesystem.agent.tool.ToolRegistry;
import com.studyback.hospitalservicesystem.agent.type.ActionType;
import com.studyback.hospitalservicesystem.agent.type.IntentType;
import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;
import com.studyback.hospitalservicesystem.agent.type.WizardStep;
import com.studyback.hospitalservicesystem.dto.QAResponse.KnowledgeSource;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Agent 编排器 — 核心管道协调者
 * <p>
 * 串联 Planner → 执行 Actions → Aggregator → Generator 的完整流程。
 *
 * <pre>
 * 用户问题
 *   ↓
 * IntentPlannerService.plan()    — LLM 意图识别 + 动作规划
 *   ↓
 * for each PlannedAction:
 *   ├─ MEMORY_RECALL → ChatMemory + UserMemory
 *   ├─ RAG_SEARCH    → EmbeddingRouter → MultiKBRetrieval
 *   └─ TOOL_CALL     → ToolRegistry.execute()
 *   ↓
 * ContextAggregator.aggregate()  — 融合所有结果
 *   ↓
 * AnswerGeneratorService.generate() — 最终 LLM 生成回答
 *   ↓
 * 持久化到 Memory → 返回 AgentResponse
 * </pre>
 */
@Slf4j
@Component
@RequiredArgsConstructor
public class AgentOrchestrator {

    private final IntentPlannerService intentPlannerService;
    private final ChatMemoryService chatMemoryService;
    private final UserMemoryService userMemoryService;
    private final EmbeddingRouterService embeddingRouterService;
    private final MultiKBRetrievalService multiKBRetrievalService;
    private final ToolRegistry toolRegistry;
    private final ContextAggregator contextAggregator;
    private final AnswerGeneratorService answerGeneratorService;
    private final ReportTool reportTool;

    /**
     * 执行完整的 Agent 管道
     *
     * @param maskedQuestion 脱敏后的用户问题
     * @param userId         用户ID
     * @param conversationId 会话ID
     * @return Agent 响应
     */
    public AgentResponse process(String maskedQuestion, Long userId, Long conversationId) {
        long startTime = System.currentTimeMillis();

        // ===== Step 1: 创建执行上下文 =====
        AgentContext ctx = new AgentContext(conversationId, userId);

        // ===== Step 2: LLM Planner — 意图识别 + 动作规划 =====
        ActionPlan plan = intentPlannerService.plan(maskedQuestion, userId, conversationId);
        ctx.setPlan(plan);
        log.info("[Agent] Step 2 Planner: intent={}, actions={}, needsHuman={}",
                plan.intent(), plan.actions().size(), plan.needsHumanService());

        // ===== Step 3: 按序执行动作 =====
        for (PlannedAction action : plan.actions()) {
            try {
                executeAction(ctx, action, maskedQuestion);
            } catch (Exception e) {
                log.error("[Agent] 执行动作 {} 失败", action.type(), e);
            }
        }

        // ===== Step 4: 生成导诊报告 =====
        String triageReport = reportTool.generateTriageReport(ctx);

        // ===== Step 5: 上下文聚合 =====
        String aggregatedContext = contextAggregator.aggregate(ctx);

        // ===== Step 6: Answer Generator =====
        GenerationResult genResult = answerGeneratorService.generate(
                maskedQuestion, aggregatedContext, triageReport);

        // ===== Step 7: 持久化向导状态 + 记忆 =====
        persistWizardState(ctx, plan);
        chatMemoryService.addMessage(conversationId, "user", maskedQuestion);
        chatMemoryService.addMessage(conversationId, "assistant", genResult.cleanAnswer());
        userMemoryService.remember(userId, "last_interaction",
                java.time.LocalDateTime.now().toString());

        // ===== Step 8: 构建响应 =====
        long costTime = System.currentTimeMillis() - startTime;

        String toolsUsed = plan.actions().stream()
                .filter(a -> a.type() == ActionType.TOOL_CALL)
                .map(PlannedAction::toolName)
                .collect(Collectors.joining(", "));
        String kbTypeUsed = ctx.getRagResults().keySet().stream()
                .map(KnowledgeBaseType::name)
                .collect(Collectors.joining(", "));

        // 提取预约结果信息
        String appointmentInfo = ctx.getBookingResult()
                .map(tr -> tr.summary())
                .orElse(null);

        return AgentResponse.builder()
                .answer(genResult.cleanAnswer())
                .sources(ctx.getAllRagSources())
                .recommendedDepartment(genResult.recommendation())
                .needHumanService(plan.needsHumanService())
                .costTimeMs(costTime)
                .actionPlanSummary(plan.reasoning())
                .kbTypeUsed(kbTypeUsed.isEmpty() ? null : kbTypeUsed)
                .toolsUsed(toolsUsed.isEmpty() ? null : toolsUsed)
                .appointmentInfo(appointmentInfo)
                .build();
    }

    /**
     * 执行单个 PlannedAction
     */
    private void executeAction(AgentContext ctx, PlannedAction action, String maskedQuestion) {
        switch (action.type()) {
            case MEMORY_RECALL -> {
                // 加载短期对话记忆
                String chatMem = chatMemoryService.getRecentMessagesText(
                        ctx.getConversationId(), 10);
                ctx.setChatMemoryText(chatMem);

                // 加载用户长期记忆
                Map<String, String> userMem = userMemoryService.recall(ctx.getUserId());
                ctx.setUserMemoryMap(userMem);

                log.info("[Agent] MEMORY_RECALL: chatMemory={}chars, userMemory={}keys",
                        chatMem.length(), userMem.size());
            }

            case RAG_SEARCH -> {
                // Embedding Router 选择知识库
                KnowledgeBaseType kbType = embeddingRouterService.route(maskedQuestion);
                List<KnowledgeSource> sources = multiKBRetrievalService.retrieve(maskedQuestion, kbType);
                ctx.addRagResults(kbType, sources);
                log.info("[Agent] RAG_SEARCH: router→{}, hit {} sources", kbType, sources.size());
            }

            case TOOL_CALL -> {
                String toolName = action.toolName();
                Map<String, Object> params = action.params();
                ToolResult result = toolRegistry.execute(toolName, params);
                ctx.addToolResult(result);
                log.info("[Agent] TOOL_CALL: {} → {}", toolName, result.success() ? "ok" : "fail");
            }
        }
    }

    /**
     * 持久化预约向导中间状态到 UserMemory
     * <p>
     * 根据 Planner 意图和工具执行结果更新向导步骤，
     * 预约成功或非预约意图时清理状态。
     */
    private void persistWizardState(AgentContext ctx, ActionPlan plan) {
        Long userId = ctx.getUserId();

        // 非预约意图 — 清理任何残留的向导状态
        if (plan.intent() != IntentType.APPOINTMENT_BOOKING) {
            userMemoryService.forget(userId, "booking_wizard_step");
            return;
        }

        // 检查是否有 book 或 cancel 操作执行成功
        for (ToolResult tr : ctx.getToolResults()) {
            if (!tr.success()) continue;

            if ("booking_book".equals(tr.toolName())) {
                // 预约成功 — 保存摘要，清理向导状态
                String summary = (String) tr.data().get("appointmentSummary");
                if (summary != null) {
                    userMemoryService.remember(userId, "last_appointment", summary);
                }
                userMemoryService.forget(userId, "booking_wizard_step");
                userMemoryService.forget(userId, "booking_department_id");
                userMemoryService.forget(userId, "booking_doctor_id");
                userMemoryService.forget(userId, "booking_date");
                userMemoryService.forget(userId, "booking_start_time");
                log.info("[Agent] 预约向导完成：预约成功，状态已清理");
                return;
            }

            if ("booking_cancel".equals(tr.toolName())) {
                userMemoryService.forget(userId, "booking_wizard_step");
                log.info("[Agent] 预约向导完成：预约已取消，状态已清理");
                return;
            }

            // 根据工具执行结果更新向导步骤
            if ("schedule_slots".equals(tr.toolName()) || "booking_slots".equals(tr.toolName())) {
                // 时段已展示 → 等待用户选时段
                userMemoryService.remember(userId, "booking_wizard_step",
                        WizardStep.AWAITING_TIME_SELECTION.name());
                if (tr.data().containsKey("doctorId")) {
                    userMemoryService.remember(userId, "booking_doctor_id",
                            String.valueOf(tr.data().get("doctorId")));
                }
            }

            if ("doctor_search".equals(tr.toolName()) || "booking_list".equals(tr.toolName())) {
                // 已在列出医生或预约列表
            }
        }

        // 如果 Planner 计划中包含了 DEPARTMENT_TOOL 但没有 SCHEDULE_TOOL/BOOKING_TOOL
        boolean hasScheduleAction = plan.actions().stream()
                .anyMatch(a -> "SCHEDULE_TOOL".equals(a.toolName())
                        || "BOOKING_TOOL".equals(a.toolName()));
        if (!hasScheduleAction) {
            // 还在选科室或刚列出医生阶段
            userMemoryService.remember(userId, "booking_wizard_step",
                    WizardStep.AWAITING_DOCTOR_SELECTION.name());
        }
    }
}
