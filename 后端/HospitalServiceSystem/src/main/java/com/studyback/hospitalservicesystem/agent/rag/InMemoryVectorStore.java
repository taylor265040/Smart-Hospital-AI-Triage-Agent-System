package com.studyback.hospitalservicesystem.agent.rag;

import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 内存向量存储实现 — 按知识库类型隔离的向量缓存
 * <p>
 * 每种 KnowledgeBaseType 维护独立的 ConcurrentHashMap<条目ID, 嵌入向量>，
 * 检索时仅搜索指定类型的条目，避免跨类型噪声。
 */
@Slf4j
@Component
public class InMemoryVectorStore implements VectorStore {

    /** 知识库类型 → (条目ID → 向量) */
    private final Map<KnowledgeBaseType, Map<Long, float[]>> stores = new ConcurrentHashMap<>();

    public InMemoryVectorStore() {
        for (KnowledgeBaseType type : KnowledgeBaseType.values()) {
            stores.put(type, new ConcurrentHashMap<>());
        }
    }

    @Override
    public void index(KnowledgeBaseType kbType, Map<Long, float[]> idVectorMap) {
        Map<Long, float[]> store = stores.get(kbType);
        store.putAll(idVectorMap);
        log.info("VectorStore 索引完成: type={}, count={}", kbType, idVectorMap.size());
    }

    @Override
    public List<ScoredId> search(KnowledgeBaseType kbType, float[] queryEmbedding, int topK, double threshold) {
        Map<Long, float[]> store = stores.get(kbType);
        if (store == null || store.isEmpty()) {
            return Collections.emptyList();
        }

        // 使用优先队列保留 topK 结果
        PriorityQueue<ScoredId> pq = new PriorityQueue<>(Comparator.comparingDouble(ScoredId::score));

        for (Map.Entry<Long, float[]> entry : store.entrySet()) {
            double similarity = cosineSimilarity(queryEmbedding, entry.getValue());
            if (similarity >= threshold) {
                pq.offer(new ScoredId(entry.getKey(), similarity));
                if (pq.size() > topK) {
                    pq.poll(); // 移除最低分的
                }
            }
        }

        // 按相似度降序排列
        List<ScoredId> results = new ArrayList<>();
        while (!pq.isEmpty()) {
            results.add(0, pq.poll());
        }
        return results;
    }

    @Override
    public void clear(KnowledgeBaseType kbType) {
        Map<Long, float[]> store = stores.get(kbType);
        if (store != null) {
            store.clear();
            log.info("VectorStore 已清空: type={}", kbType);
        }
    }

    @Override
    public void clearAll() {
        for (KnowledgeBaseType type : KnowledgeBaseType.values()) {
            stores.get(type).clear();
        }
        log.info("VectorStore 已全部清空");
    }

    @Override
    public int size(KnowledgeBaseType kbType) {
        Map<Long, float[]> store = stores.get(kbType);
        return store != null ? store.size() : 0;
    }

    @Override
    public int totalSize() {
        return stores.values().stream().mapToInt(Map::size).sum();
    }

    @Override
    public float[] getCentroid(KnowledgeBaseType kbType) {
        Map<Long, float[]> store = stores.get(kbType);
        if (store == null || store.isEmpty()) {
            return null;
        }

        Collection<float[]> vectors = store.values();
        // 取第一个向量的维度
        int dim = vectors.iterator().next().length;
        float[] centroid = new float[dim];
        int count = 0;

        for (float[] vec : vectors) {
            for (int i = 0; i < dim; i++) {
                centroid[i] += vec[i];
            }
            count++;
        }

        for (int i = 0; i < dim; i++) {
            centroid[i] /= count;
        }

        return centroid;
    }

    // ==================== 私有方法 ====================

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
