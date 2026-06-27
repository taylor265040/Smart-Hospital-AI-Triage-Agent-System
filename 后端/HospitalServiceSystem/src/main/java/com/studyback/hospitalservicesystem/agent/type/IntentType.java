package com.studyback.hospitalservicesystem.agent.type;

/**
 * 用户意图类型枚举
 */
public enum IntentType {

    /** 症状描述/导诊请求（如"我咳嗽"、"头疼三天"） */
    SYMPTOM_INQUIRY("症状问询"),

    /** 科室查询（如"呼吸内科在哪"、"有哪些科室"） */
    DEPARTMENT_QUERY("科室查询"),

    /** 医生查询（如"张伟医生怎么样"、"心内科有哪些专家"） */
    DOCTOR_QUERY("医生查询"),

    /** 挂号指引（如"怎么挂号"、"预约流程"） */
    REGISTRATION("挂号指引"),

    /** 医保查询（如"医保怎么报销"、"报销需要什么材料"） */
    INSURANCE_QUERY("医保查询"),

    /** 导航查询（如"发热门诊在哪里"、"CT在几楼"） */
    NAVIGATION_QUERY("导航查询"),

    /** 转人工客服 */
    HUMAN_SERVICE("转人工"),

    /** 一般性FAQ */
    GENERAL_FAQ("一般咨询"),

    /** 预约挂号操作（查号源、预约、取消、查我的预约） */
    APPOINTMENT_BOOKING("预约操作"),

    /** 问候/闲聊 */
    GREETING("问候");

    private final String displayName;

    IntentType(String displayName) {
        this.displayName = displayName;
    }

    public String getDisplayName() {
        return displayName;
    }
}
