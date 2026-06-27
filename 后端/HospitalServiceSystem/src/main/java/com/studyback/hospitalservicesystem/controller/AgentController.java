package com.studyback.hospitalservicesystem.controller;

import com.studyback.hospitalservicesystem.agent.dto.AgentResponse;
import com.studyback.hospitalservicesystem.agent.orchestrator.AgentOrchestrator;
import com.studyback.hospitalservicesystem.agent.rag.MultiKBRetrievalService;
import com.studyback.hospitalservicesystem.agent.tool.ToolRegistry;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.dto.QARequest;
import com.studyback.hospitalservicesystem.service.PrivacyProtectionService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.web.bind.annotation.*;

import java.util.Map;

/**
 * Agent 智能导诊控制器（v2）
 * <p>
 * 基于 LangChain4j Agent 架构的新版智能问答端点：
 * LLM Planner → Multi-Action Planning → Tool Calling → Embedding Router → Answer Generator
 * <p>
 * 旧版端点 /api/qa/ask 保持不变，此为新版并行运行。
 */
@Slf4j
@Tag(name = "Agent智能导诊（v2）", description = "基于Agent架构的新版智能导诊：意图识别 → 多动作规划 → 工具调用 → 知识库路由 → 生成回答")
@RestController
@RequestMapping("/api/v2/qa")
@RequiredArgsConstructor
@ConditionalOnProperty(prefix = "agent", name = "enabled", havingValue = "true", matchIfMissing = true)
public class AgentController {

    private final AgentOrchestrator agentOrchestrator;
    private final PrivacyProtectionService privacyProtectionService;
    private final MultiKBRetrievalService multiKBRetrievalService;
    private final ToolRegistry toolRegistry;

    @Operation(summary = "Agent智能问答",
            description = "用户发送问题，Agent自动进行意图识别、动作规划、工具调用和知识检索，生成回答")
    @PostMapping("/ask")
    public Result<AgentResponse> ask(@Valid @RequestBody QARequest request) {
        long startTime = System.currentTimeMillis();

        // Step 1: 隐私脱敏（复用已有服务）
        PrivacyProtectionService.MaskResult maskResult =
                privacyProtectionService.maskWithReport(request.getQuestion());
        log.info("[AgentV2] Step 1 隐私脱敏: hasPII={}", maskResult.hasPII());

        // Step 2: Agent 管道
        AgentResponse response = agentOrchestrator.process(
                maskResult.maskedText(), request.getUserId(), request.getConversationId());

        log.info("[AgentV2] 智能问答完成 — 总耗时: {}ms, 推荐科室: {}, 动作: {}",
                response.costTimeMs(),
                response.recommendedDepartment() != null
                        ? response.recommendedDepartment().getDepartmentName() : "无",
                response.actionPlanSummary());

        return Result.ok(response);
    }

    @Operation(summary = "Agent健康检查", description = "返回各组件状态")
    @GetMapping("/health")
    public Result<Map<String, Object>> health() {
        Map<String, Object> status = Map.of(
                "agentEnabled", true,
                "totalIndexed", multiKBRetrievalService.getTotalIndexedCount(),
                "kbStats", multiKBRetrievalService.getIndexStats(),
                "registeredTools", toolRegistry.getRegisteredToolNames()
        );
        return Result.ok(status);
    }

    @Operation(summary = "刷新知识库索引", description = "刷新所有知识库的向量索引")
    @PostMapping("/refresh-index")
    public Result<String> refreshIndex() {
        multiKBRetrievalService.refreshAllIndexes();
        return Result.ok("多知识库索引已刷新，总条目数：" + multiKBRetrievalService.getTotalIndexedCount());
    }
}
