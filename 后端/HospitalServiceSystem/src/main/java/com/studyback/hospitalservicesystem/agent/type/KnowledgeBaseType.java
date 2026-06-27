package com.studyback.hospitalservicesystem.agent.type;

/**
 * 知识库类型枚举 — 对应5个子知识库
 */
public enum KnowledgeBaseType {

    /** 常见FAQ：开放时间、挂号流程、就诊须知 */
    FAQ("常见问题"),

    /** 医生知识库：医生简介、职称、擅长领域 */
    DOCTOR("医生信息"),

    /** 科室知识库：科室介绍、诊疗范围 */
    DEPARTMENT("科室信息"),

    /** 医保知识库：医保政策、报销规则 */
    INSURANCE("医保政策"),

    /** 导航知识库：楼层导航、门诊位置 */
    NAVIGATION("医院导航");

    private final String displayName;

    KnowledgeBaseType(String displayName) {
        this.displayName = displayName;
    }

    public String getDisplayName() {
        return displayName;
    }

    /**
     * 从 kbType 字段值映射到枚举（优先使用）
     */
    public static KnowledgeBaseType fromKbType(String kbType) {
        if (kbType == null || kbType.isBlank()) return FAQ;
        try {
            return valueOf(kbType.trim().toUpperCase());
        } catch (IllegalArgumentException e) {
            return FAQ;
        }
    }

    /**
     * 从旧的 category 字段映射到新的 KB 类型（兼容过渡期）
     */
    public static KnowledgeBaseType fromCategory(String category) {
        if (category == null) return FAQ;
        return switch (category) {
            case "医生信息", "医生" -> DOCTOR;
            case "科室介绍", "科室位置" -> DEPARTMENT;
            case "医保政策", "医保" -> INSURANCE;
            case "导航", "楼层导航", "检查指引" -> NAVIGATION;
            default -> FAQ;
        };
    }
}
