package com.studyback.hospitalservicesystem.agent.tool;

import com.studyback.hospitalservicesystem.agent.dto.AgentContext;
import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import com.studyback.hospitalservicesystem.agent.type.IntentType;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 导诊报告工具
 * <p>
 * 汇总 Agent 执行上下文，生成结构化的导诊摘要，供 Answer Generator 使用。
 */
@Slf4j
@Component("agentReportTool")
public class ReportTool {

    /**
     * 生成导诊报告
     */
    public String generateTriageReport(AgentContext ctx) {
        StringBuilder report = new StringBuilder();
        report.append("【导诊摘要】\n");

        // 意图
        IntentType intent = ctx.getPlan() != null ? ctx.getPlan().intent() : null;
        report.append("意图：").append(intent != null ? intent.getDisplayName() : "未知").append("\n");

        // 记忆
        if (ctx.getChatMemoryText() != null && !ctx.getChatMemoryText().isBlank()) {
            report.append("\n【历史对话】\n").append(ctx.getChatMemoryText()).append("\n");
        }
        if (ctx.getUserMemoryMap() != null && !ctx.getUserMemoryMap().isEmpty()) {
            report.append("\n【用户画像】\n");
            ctx.getUserMemoryMap().forEach((k, v) ->
                    report.append("  ").append(k).append(": ").append(v).append("\n"));
        }

        // 工具结果
        if (!ctx.getToolResults().isEmpty()) {
            report.append("\n【实时查询结果】\n");
            for (ToolResult tr : ctx.getToolResults()) {
                report.append("  [").append(tr.toolName()).append("] ")
                        .append(tr.success() ? "✓ " : "✗ ")
                        .append(tr.summary()).append("\n");
            }
        }

        // RAG 结果
        if (!ctx.getRagResults().isEmpty()) {
            report.append("\n【知识库参考】\n");
            ctx.getRagResults().forEach((type, sources) -> {
                if (!sources.isEmpty()) {
                    report.append("  ").append(type.getDisplayName()).append("（")
                            .append(sources.size()).append("条）：\n");
                    sources.forEach(s -> report.append("    - ").append(s.getQuestion()).append("\n"));
                }
            });
        }

        return report.toString();
    }

    /**
     * 提取关键词（供 LLM 使用）
     */
    public String extractKeywords(AgentContext ctx) {
        return ctx.getToolResults().stream()
                .filter(ToolResult::success)
                .map(ToolResult::summary)
                .collect(Collectors.joining("；"));
    }
}
