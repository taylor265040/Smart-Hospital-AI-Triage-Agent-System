package com.studyback.hospitalservicesystem.agent.dto;

import com.studyback.hospitalservicesystem.agent.type.ActionType;

import java.util.Map;

/**
 * Planner 输出的单个动作
 */
public record PlannedAction(
        /** 动作类型 */
        ActionType type,
        /** 工具名称（仅 TOOL_CALL 时有值） */
        String toolName,
        /** 工具参数（仅 TOOL_CALL 时有值） */
        Map<String, Object> params,
        /** 动作描述（供调试用） */
        String description
) {
    public static PlannedAction memoryRecall() {
        return new PlannedAction(ActionType.MEMORY_RECALL, null, null, "读取对话记忆和用户长期记忆");
    }

    public static PlannedAction ragSearch() {
        return new PlannedAction(ActionType.RAG_SEARCH, null, null, "知识库检索");
    }

    public static PlannedAction toolCall(String toolName, Map<String, Object> params, String description) {
        return new PlannedAction(ActionType.TOOL_CALL, toolName, params, description);
    }
}
