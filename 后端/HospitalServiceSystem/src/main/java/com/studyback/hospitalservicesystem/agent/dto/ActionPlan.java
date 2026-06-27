package com.studyback.hospitalservicesystem.agent.dto;

import com.studyback.hospitalservicesystem.agent.type.IntentType;

import java.util.List;

/**
 * LLM Planner 输出的完整动作计划
 */
public record ActionPlan(
        /** 识别出的用户意图 */
        IntentType intent,
        /** 需要按序执行的动作列表 */
        List<PlannedAction> actions,
        /** 是否需要转人工客服 */
        boolean needsHumanService,
        /** Planner 的推理过程（调试用） */
        String reasoning
) {
    /**
     * 创建默认计划 — 当 LLM Planner 失败时的降级方案
     */
    public static ActionPlan fallback() {
        return new ActionPlan(
                IntentType.GENERAL_FAQ,
                List.of(PlannedAction.ragSearch()),
                false,
                "降级方案：直接走 RAG 检索"
        );
    }
}
