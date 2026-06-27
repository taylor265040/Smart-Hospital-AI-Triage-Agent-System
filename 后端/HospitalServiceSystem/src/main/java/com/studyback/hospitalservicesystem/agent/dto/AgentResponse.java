package com.studyback.hospitalservicesystem.agent.dto;

import com.studyback.hospitalservicesystem.dto.QAResponse.DepartmentRecommendation;
import com.studyback.hospitalservicesystem.dto.QAResponse.KnowledgeSource;
import lombok.Builder;
import java.util.List;

/**
 * Agent 最终响应 — 扩展自旧版 QAResponse 的字段
 */
@Builder
public record AgentResponse(
        /** AI 回答 */
        String answer,
        /** 检索到的知识来源 */
        List<KnowledgeSource> sources,
        /** 推荐的科室 */
        DepartmentRecommendation recommendedDepartment,
        /** 是否需要转人工 */
        boolean needHumanService,
        /** 总耗时（毫秒） */
        long costTimeMs,
        /** Planner 推理摘要（调试用） */
        String actionPlanSummary,
        /** 使用的知识库类型 */
        String kbTypeUsed,
        /** 调用的工具列表 */
        String toolsUsed,

        /** 预约操作结果（JSON 摘要，当 Agent 执行了预约/取消时返回） */
        String appointmentInfo
) {}
