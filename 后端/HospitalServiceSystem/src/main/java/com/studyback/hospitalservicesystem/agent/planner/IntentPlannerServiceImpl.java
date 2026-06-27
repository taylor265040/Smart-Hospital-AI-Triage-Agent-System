package com.studyback.hospitalservicesystem.agent.planner;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.studyback.hospitalservicesystem.agent.config.AgentProperties;
import com.studyback.hospitalservicesystem.agent.dto.ActionPlan;
import com.studyback.hospitalservicesystem.agent.dto.PlannedAction;
import com.studyback.hospitalservicesystem.agent.memory.ChatMemoryService;
import com.studyback.hospitalservicesystem.agent.type.ActionType;
import com.studyback.hospitalservicesystem.agent.type.IntentType;
import dev.langchain4j.data.message.AiMessage;
import dev.langchain4j.data.message.SystemMessage;
import dev.langchain4j.data.message.UserMessage;
import dev.langchain4j.model.chat.ChatLanguageModel;
import dev.langchain4j.model.output.Response;
import jakarta.annotation.PostConstruct;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.Resource;
import org.springframework.stereotype.Service;
import org.springframework.util.StreamUtils;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * LLM Planner 实现
 * <p>
 * 使用 LangChain4j ChatLanguageModel 调用 Qwen，规划 Agent 执行计划。
 * 支持最多 N 次重试（JSON 解析失败时）。
 */
@Slf4j
@Service
@RequiredArgsConstructor
public class IntentPlannerServiceImpl implements IntentPlannerService {

    private final ChatLanguageModel chatLanguageModel;
    private final ObjectMapper objectMapper;
    private final AgentProperties agentProperties;
    private final ChatMemoryService chatMemoryService;

    @Value("classpath:prompts/planner-system-prompt.txt")
    private Resource plannerPromptResource;

    private String plannerSystemPrompt;

    @PostConstruct
    private void loadPrompt() {
        try {
            this.plannerSystemPrompt = StreamUtils.copyToString(
                    plannerPromptResource.getInputStream(), StandardCharsets.UTF_8).trim();
        } catch (IOException e) {
            throw new IllegalStateException("加载 Planner Prompt 文件失败", e);
        }
    }

    @Override
    public ActionPlan plan(String maskedQuestion, Long userId, Long conversationId) {
        // 预加载对话历史，让 Planner 感知多轮对话上下文
        String chatHistory = chatMemoryService.getRecentMessagesText(conversationId, 10);

        int maxRetries = agentProperties.getPlannerMaxRetries();
        String lastError = null;

        for (int attempt = 0; attempt <= maxRetries; attempt++) {
            try {
                String json = callLLMForPlan(maskedQuestion, userId, conversationId,
                        chatHistory, lastError);
                return parseActionPlan(json);
            } catch (Exception e) {
                lastError = e.getMessage();
                log.warn("Planner JSON 解析失败 (attempt {}/{}): {}", attempt + 1, maxRetries + 1, lastError);
                if (attempt >= maxRetries) {
                    log.error("Planner 重试耗尽，使用降级方案");
                }
            }
        }

        return ActionPlan.fallback();
    }

    private String callLLMForPlan(String question, Long userId, Long conversationId,
                                   String chatHistory, String previousError) {
        List<dev.langchain4j.data.message.ChatMessage> messages = new ArrayList<>();
        messages.add(SystemMessage.from(plannerSystemPrompt));

        // 带对话历史和记忆上下文
        StringBuilder userMsg = new StringBuilder();
        if (chatHistory != null && !chatHistory.isBlank()) {
            userMsg.append("【对话历史（最近几轮）】\n")
                    .append(chatHistory).append("\n\n");
        }
        userMsg.append("【用户问题】\n").append(question).append("\n");
        userMsg.append("【当前日期】").append(java.time.LocalDate.now()).append("\n");
        userMsg.append("【会话ID】").append(conversationId).append("\n");
        userMsg.append("【用户ID】").append(userId).append("\n");
        if (previousError != null) {
            userMsg.append("\n上次输出的JSON解析失败，错误：").append(previousError)
                    .append("。请确保输出合法JSON。\n");
        }

        messages.add(UserMessage.from(userMsg.toString()));
        Response<AiMessage> response = chatLanguageModel.generate(messages);
        return response.content().text();
    }

    private ActionPlan parseActionPlan(String json) throws Exception {
        // 清理可能的 markdown 代码块标记
        String cleaned = json.trim();
        if (cleaned.startsWith("```")) {
            cleaned = cleaned.replaceAll("^```(?:json)?\\s*", "");
            cleaned = cleaned.replaceAll("\\s*```$", "");
        }

        JsonNode root = objectMapper.readTree(cleaned);

        String intentStr = root.path("intent").asText("GENERAL_FAQ");
        IntentType intent;
        try {
            intent = IntentType.valueOf(intentStr);
        } catch (IllegalArgumentException e) {
            intent = IntentType.GENERAL_FAQ;
        }

        List<PlannedAction> actions = new ArrayList<>();
        JsonNode actionsNode = root.path("actions");
        if (actionsNode.isArray()) {
            for (JsonNode actionNode : actionsNode) {
                String typeStr = actionNode.path("type").asText();
                ActionType type;
                try {
                    type = ActionType.valueOf(typeStr);
                } catch (IllegalArgumentException e) {
                    continue;
                }

                String toolName = actionNode.path("toolName").asText(null);
                @SuppressWarnings("unchecked")
                Map<String, Object> params = actionNode.has("params")
                        ? objectMapper.convertValue(actionNode.get("params"), Map.class)
                        : null;

                actions.add(new PlannedAction(type, toolName, params,
                        actionNode.path("description").asText(null)));
            }
        }

        boolean needsHuman = root.path("needsHumanService").asBoolean(false);
        String reasoning = root.path("reasoning").asText("");

        return new ActionPlan(intent, actions, needsHuman, reasoning);
    }
}
