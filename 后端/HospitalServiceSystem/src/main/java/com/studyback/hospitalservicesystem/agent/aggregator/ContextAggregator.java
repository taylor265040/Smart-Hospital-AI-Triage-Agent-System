package com.studyback.hospitalservicesystem.agent.aggregator;

import com.studyback.hospitalservicesystem.agent.dto.AgentContext;
import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;
import com.studyback.hospitalservicesystem.dto.QAResponse.KnowledgeSource;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * 上下文聚合器
 * <p>
 * 将 Memory + Tool + RAG 三类结果融合为统一的结构化上下文文本，
 * 供 Answer Generator 构建最终 Prompt。
 */
@Slf4j
@Component
public class ContextAggregator {

    /**
     * 聚合所有上下文
     */
    public String aggregate(AgentContext ctx) {
        StringBuilder sb = new StringBuilder();

        // 1. 用户记忆
        appendMemory(sb, ctx);

        // 2. 工具查询结果
        appendToolResults(sb, ctx);

        // 3. RAG 知识检索结果
        appendRagResults(sb, ctx);

        String result = sb.toString();
        log.info("ContextAggregator 生成上下文: {} chars", result.length());
        return result;
    }

    private void appendMemory(StringBuilder sb, AgentContext ctx) {
        boolean hasMemory = false;

        if (ctx.getChatMemoryText() != null && !ctx.getChatMemoryText().isBlank()) {
            sb.append("## 历史对话\n");
            sb.append(ctx.getChatMemoryText()).append("\n");
            hasMemory = true;
        }

        if (ctx.getUserMemoryMap() != null && !ctx.getUserMemoryMap().isEmpty()) {
            sb.append("## 用户画像\n");
            for (Map.Entry<String, String> entry : ctx.getUserMemoryMap().entrySet()) {
                String label = switch (entry.getKey()) {
                    case "medical_history" -> "病史";
                    case "preferred_doctor" -> "偏好医生";
                    case "last_department" -> "上次就诊科室";
                    case "last_appointment" -> "上次预约";
                    default -> entry.getKey();
                };
                sb.append("- ").append(label).append(": ").append(entry.getValue()).append("\n");
            }
            hasMemory = true;
        }

        if (hasMemory) sb.append("\n");
    }

    private void appendToolResults(StringBuilder sb, AgentContext ctx) {
        if (ctx.getToolResults().isEmpty()) return;

        sb.append("## 实时查询结果\n");
        for (ToolResult tr : ctx.getToolResults()) {
            sb.append("### ").append(tr.toolName()).append("\n");
            sb.append(tr.summary()).append("\n");
            if (tr.data() != null && !tr.data().isEmpty()) {
                // 简单格式化数据
                tr.data().forEach((k, v) -> {
                    if (!k.equals("count") && v != null) {
                        sb.append("- ").append(k).append(": ").append(v).append("\n");
                    }
                });
            }
            sb.append("\n");
        }
    }

    private void appendRagResults(StringBuilder sb, AgentContext ctx) {
        if (ctx.getRagResults().isEmpty()) return;

        sb.append("## 知识库参考\n");
        for (Map.Entry<KnowledgeBaseType, List<KnowledgeSource>> entry : ctx.getRagResults().entrySet()) {
            if (entry.getValue().isEmpty()) continue;
            sb.append("### ").append(entry.getKey().getDisplayName()).append("\n");
            for (KnowledgeSource ks : entry.getValue()) {
                sb.append("Q: ").append(ks.getQuestion()).append("\n");
                sb.append("A: ").append(ks.getAnswer()).append("\n");
                sb.append("(相似度: ").append(String.format("%.2f", ks.getSimilarity())).append(")\n\n");
            }
        }
    }
}
