package com.studyback.hospitalservicesystem.agent.type;

/**
 * Agent 动作类型枚举
 */
public enum ActionType {

    /** 读取记忆（ChatMemory + UserMemory） */
    MEMORY_RECALL("记忆读取"),

    /** 调用业务工具（DepartmentTool / DoctorTool / ScheduleTool / RegisterTool / ReportTool） */
    TOOL_CALL("工具调用"),

    /** RAG 知识库检索（经 Embedding Router 路由到对应知识库） */
    RAG_SEARCH("知识检索");

    private final String displayName;

    ActionType(String displayName) {
        this.displayName = displayName;
    }

    public String getDisplayName() {
        return displayName;
    }
}
