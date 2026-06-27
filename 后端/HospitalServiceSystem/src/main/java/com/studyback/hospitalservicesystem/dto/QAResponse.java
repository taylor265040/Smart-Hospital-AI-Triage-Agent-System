package com.studyback.hospitalservicesystem.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 智能问答响应
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class QAResponse {

    /** AI 回答 */
    private String answer;

    /** 检索到的知识库来源（用于追溯答案依据） */
    private List<KnowledgeSource> sources;

    /** 推荐的科室（当用户描述症状时自动推荐） */
    private DepartmentRecommendation recommendedDepartment;

    /** 是否需要转人工客服（前端据此显示\"转人工\"按钮） */
    private Boolean needHumanService;

    /** 问答用时（毫秒） */
    private Long costTimeMs;

    /**
     * 知识来源
     */
    @Data
    @Builder
    @NoArgsConstructor
    @AllArgsConstructor
    public static class KnowledgeSource {
        private Long id;
        private String question;
        private String answer;
        private Double similarity;
    }

    /**
     * 科室推荐（用于导诊跳转预约）
     */
    @Data
    @Builder
    @NoArgsConstructor
    @AllArgsConstructor
    public static class DepartmentRecommendation {
        /** 推荐科室ID */
        private Long departmentId;
        /** 推荐科室名称 */
        private String departmentName;
        /** 推荐理由 */
        private String reason;
    }

}
