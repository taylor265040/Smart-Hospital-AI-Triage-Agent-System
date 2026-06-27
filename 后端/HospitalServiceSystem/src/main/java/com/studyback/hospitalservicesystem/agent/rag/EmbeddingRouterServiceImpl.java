package com.studyback.hospitalservicesystem.agent.rag;

import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.*;

/**
 * Embedding Router 实现
 * <p>
 * 使用质心路由策略：
 * 1. 启动时为每个知识库类型预计算质心向量（所有条目的平均向量）
 * 2. 查询时计算 query embedding 与各质心的余弦相似度
 * 3. 选择相似度最高的知识库（低于阈值时默认 FAQ）
 */
@Slf4j
@Service
@RequiredArgsConstructor
public class EmbeddingRouterServiceImpl implements EmbeddingRouterService {

    private final VectorStore vectorStore;
    private final MultiKBRetrievalService retrievalService;

    /** 路由的最小相似度阈值（低于此值回退到 FAQ） */
    private static final double ROUTE_THRESHOLD = 0.35;

    @Override
    public KnowledgeBaseType route(String maskedQuestion) {
        Map<KnowledgeBaseType, Double> scores = routeWithScores(maskedQuestion);
        KnowledgeBaseType best = scores.entrySet().stream()
                .max(Map.Entry.comparingByValue())
                .map(Map.Entry::getKey)
                .orElse(KnowledgeBaseType.FAQ);

        double bestScore = scores.getOrDefault(best, 0.0);
        if (bestScore < ROUTE_THRESHOLD) {
            log.info("EmbeddingRouter: 最佳匹配 {} (score={}) 低于阈值 {}，回退到 FAQ",
                    best, String.format("%.4f", bestScore), ROUTE_THRESHOLD);
            return KnowledgeBaseType.FAQ;
        }

        log.info("EmbeddingRouter: 路由到 {} (score={})", best, String.format("%.4f", bestScore));
        return best;
    }

    @Override
    public Map<KnowledgeBaseType, Double> routeWithScores(String maskedQuestion) {
        float[] queryEmbedding = retrievalService.embedQuery(maskedQuestion);

        Map<KnowledgeBaseType, Double> scores = new LinkedHashMap<>();
        for (KnowledgeBaseType kbType : KnowledgeBaseType.values()) {
            float[] centroid = vectorStore.getCentroid(kbType);
            if (centroid != null) {
                double similarity = cosineSimilarity(queryEmbedding, centroid);
                scores.put(kbType, similarity);
            } else {
                scores.put(kbType, 0.0);
            }
        }

        return scores;
    }

    private double cosineSimilarity(float[] a, float[] b) {
        double dot = 0, normA = 0, normB = 0;
        for (int i = 0; i < a.length; i++) {
            dot += a[i] * b[i];
            normA += a[i] * a[i];
            normB += b[i] * b[i];
        }
        return dot / (Math.sqrt(normA) * Math.sqrt(normB));
    }
}
