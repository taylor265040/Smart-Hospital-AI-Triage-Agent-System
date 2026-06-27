package com.studyback.hospitalservicesystem.agent.dto;

import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;
import com.studyback.hospitalservicesystem.dto.QAResponse.KnowledgeSource;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Agent 执行上下文 — 在整个管道中流转，收集各阶段结果
 */
public class AgentContext {

    private final Long conversationId;
    private final Long userId;
    private ActionPlan plan;

    /** 短期对话记忆文本 */
    private String chatMemoryText;
    /** 用户长期记忆 KV */
    private Map<String, String> userMemoryMap;

    /** 工具执行结果列表 */
    private final List<ToolResult> toolResults = new ArrayList<>();
    /** RAG 检索结果（可能来自多个知识库） */
    private final Map<KnowledgeBaseType, List<KnowledgeSource>> ragResults = new LinkedHashMap<>();

    public AgentContext(Long conversationId, Long userId) {
        this.conversationId = conversationId;
        this.userId = userId;
    }

    public void setPlan(ActionPlan plan) { this.plan = plan; }
    public ActionPlan getPlan() { return plan; }

    public void setChatMemoryText(String text) { this.chatMemoryText = text; }
    public String getChatMemoryText() { return chatMemoryText; }

    public void setUserMemoryMap(Map<String, String> map) { this.userMemoryMap = map; }
    public Map<String, String> getUserMemoryMap() { return userMemoryMap; }

    public void addToolResult(ToolResult r) { toolResults.add(r); }
    public List<ToolResult> getToolResults() { return toolResults; }

    public void addRagResults(KnowledgeBaseType type, List<KnowledgeSource> sources) {
        ragResults.put(type, sources);
    }
    public Map<KnowledgeBaseType, List<KnowledgeSource>> getRagResults() { return ragResults; }

    /** 合并所有 RAG 结果为单一列表 */
    public List<KnowledgeSource> getAllRagSources() {
        List<KnowledgeSource> all = new ArrayList<>();
        ragResults.values().forEach(all::addAll);
        return all;
    }

    public Long getConversationId() { return conversationId; }
    public Long getUserId() { return userId; }

    /** 获取第一个成功的 BOOKING_TOOL 结果 */
    public java.util.Optional<ToolResult> getBookingResult() {
        return toolResults.stream()
                .filter(tr -> tr.toolName().startsWith("booking_") && tr.success())
                .findFirst();
    }
}
