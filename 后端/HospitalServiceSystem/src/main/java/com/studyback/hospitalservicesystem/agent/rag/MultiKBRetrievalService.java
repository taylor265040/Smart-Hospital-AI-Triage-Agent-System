package com.studyback.hospitalservicesystem.agent.rag;

import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;
import com.studyback.hospitalservicesystem.dto.QAResponse.KnowledgeSource;
import com.studyback.hospitalservicesystem.entity.KnowledgeBase;

import java.util.List;
import java.util.Map;

/**
 * 多知识库 RAG 检索服务
 * <p>
 * 支持按 KnowledgeBaseType 分别索引和检索。
 */
public interface MultiKBRetrievalService {

    /**
     * 初始化所有知识库索引（应用启动时调用）
     */
    void initializeAllIndexes();

    /**
     * 在指定知识库类型中检索
     *
     * @param query  用户问题（已脱敏）
     * @param kbType 目标知识库类型
     * @return Top-K 相关知识来源
     */
    List<KnowledgeSource> retrieve(String query, KnowledgeBaseType kbType);

    /**
     * 获取 query 的嵌入向量
     */
    float[] embedQuery(String query);

    /**
     * 获取各知识库的已索引条目数
     */
    Map<KnowledgeBaseType, Integer> getIndexStats();

    /**
     * 获取所有知识库总索引数
     */
    int getTotalIndexedCount();

    /**
     * 刷新所有知识库索引
     */
    void refreshAllIndexes();
}
