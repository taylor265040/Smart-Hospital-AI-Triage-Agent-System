package com.studyback.hospitalservicesystem.agent.planner;

import com.studyback.hospitalservicesystem.agent.dto.ActionPlan;

/**
 * LLM Planner — 意图识别与动作规划（第一层路由）
 * <p>
 * 调用 LLM 分析用户问题，输出结构化的 ActionPlan（JSON）。
 */
public interface IntentPlannerService {

    /**
     * 分析用户问题并生成执行计划
     *
     * @param maskedQuestion 脱敏后的用户问题
     * @param userId         用户ID（供记忆上下文使用）
     * @param conversationId 会话ID
     * @return 动作计划
     */
    ActionPlan plan(String maskedQuestion, Long userId, Long conversationId);
}
