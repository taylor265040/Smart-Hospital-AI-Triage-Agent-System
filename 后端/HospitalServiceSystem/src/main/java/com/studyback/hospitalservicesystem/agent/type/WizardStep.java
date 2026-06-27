package com.studyback.hospitalservicesystem.agent.type;

/**
 * 预约向导步骤枚举
 * <p>
 * 用于持久化多轮预约对话的中间状态到 UserMemory。
 * Planner 优先根据 ChatMemory 中的对话历史推断当前步骤，
 * 此枚举作为辅助的确定性回退方案。
 */
public enum WizardStep {

    /** 无活跃向导 */
    NONE,

    /** 等待用户选择医生（已确定科室和日期） */
    AWAITING_DOCTOR_SELECTION,

    /** 等待用户选择时段（已确定医生） */
    AWAITING_TIME_SELECTION,

    /** 等待用户确认预约（所有信息已齐备） */
    AWAITING_CONFIRMATION
}
