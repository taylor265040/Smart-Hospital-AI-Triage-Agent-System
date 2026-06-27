package com.studyback.hospitalservicesystem.agent.rag;

import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;

/**
 * Embedding Router（第二层路由）
 * <p>
 * 当 Planner 判定需要使用 RAG 时，由 Embedding Router 根据查询向量
 * 与各知识库质心向量的相似度，自动选择最匹配的知识库。
 */
public interface EmbeddingRouterService {

    /**
     * 路由用户查询到最匹配的知识库类型
     *
     * @param maskedQuestion 脱敏后的用户问题
     * @return 最佳匹配的知识库类型（所有相似度都低于阈值时返回 FAQ 作为默认）
     */
    KnowledgeBaseType route(String maskedQuestion);

    /**
     * 路由并返回各知识库的相似度得分（调试用）
     *
     * @param maskedQuestion 脱敏后的用户问题
     * @return 知识库类型 → 相似度得分
     */
    java.util.Map<KnowledgeBaseType, Double> routeWithScores(String maskedQuestion);
}
