package com.studyback.hospitalservicesystem.agent.rag;

import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;

import java.util.List;
import java.util.Map;

/**
 * 向量存储接口 — 支持多知识库类型的向量索引和检索
 */
public interface VectorStore {

    /**
     * 索引一批知识条目
     *
     * @param kbType     知识库类型
     * @param idVectorMap 条目ID → 嵌入向量
     */
    void index(KnowledgeBaseType kbType, Map<Long, float[]> idVectorMap);

    /**
     * 在指定知识库类型中检索 Top-K 最相似条目
     *
     * @param kbType         知识库类型
     * @param queryEmbedding 查询向量
     * @param topK           返回 Top-K 条
     * @param threshold      相似度阈值
     * @return 按相似度降序排列的条目ID列表
     */
    List<ScoredId> search(KnowledgeBaseType kbType, float[] queryEmbedding, int topK, double threshold);

    /**
     * 清空指定知识库类型的索引
     */
    void clear(KnowledgeBaseType kbType);

    /**
     * 清空全部索引
     */
    void clearAll();

    /**
     * 获取某类型已索引条目数
     */
    int size(KnowledgeBaseType kbType);

    /**
     * 获取总索引条目数
     */
    int totalSize();

    /**
     * 获取指定知识库类型的质心向量（所有条目向量的平均值）
     * 用于 Embedding Router 路由决策
     */
    float[] getCentroid(KnowledgeBaseType kbType);

    /**
     * 带相似度分数的条目ID
     */
    record ScoredId(Long id, double score) {}
}
