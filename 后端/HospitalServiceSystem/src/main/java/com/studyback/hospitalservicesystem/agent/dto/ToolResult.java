package com.studyback.hospitalservicesystem.agent.dto;

import java.util.Map;

/**
 * 工具执行结果 — 统一返回类型
 */
public record ToolResult(
        /** 工具名称 */
        String toolName,
        /** 执行是否成功 */
        boolean success,
        /** 人类可读的结果摘要 */
        String summary,
        /** 结构化数据 */
        Map<String, Object> data
) {
    public static ToolResult ok(String toolName, String summary, Map<String, Object> data) {
        return new ToolResult(toolName, true, summary, data);
    }

    public static ToolResult fail(String toolName, String error) {
        return new ToolResult(toolName, false, error, Map.of("error", error));
    }
}
