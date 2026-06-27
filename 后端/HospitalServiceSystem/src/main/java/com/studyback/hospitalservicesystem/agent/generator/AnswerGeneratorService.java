package com.studyback.hospitalservicesystem.agent.generator;

import com.studyback.hospitalservicesystem.dto.QAResponse.DepartmentRecommendation;

/**
 * Answer Generator — 基于聚合后的上下文生成最终回答
 */
public interface AnswerGeneratorService {

    /**
     * AI 生成结果
     */
    record GenerationResult(String cleanAnswer, DepartmentRecommendation recommendation) {}

    /**
     * 生成最终回答
     *
     * @param question      用户原始问题（已脱敏）
     * @param context       聚合后的上下文
     * @param triageReport  导诊报告（可选）
     * @return 回答 + 科室推荐
     */
    GenerationResult generate(String question, String context, String triageReport);
}
