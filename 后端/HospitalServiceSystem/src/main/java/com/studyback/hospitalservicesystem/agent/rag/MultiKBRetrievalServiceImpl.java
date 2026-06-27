package com.studyback.hospitalservicesystem.agent.rag;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.studyback.hospitalservicesystem.agent.type.KnowledgeBaseType;
import com.studyback.hospitalservicesystem.config.DashScopeConfig;
import com.studyback.hospitalservicesystem.dto.QAResponse.KnowledgeSource;
import com.studyback.hospitalservicesystem.entity.KnowledgeBase;
import com.studyback.hospitalservicesystem.service.KnowledgeBaseService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestClient;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

/**
 * 多知识库 RAG 检索服务实现
 * <p>
 * 使用 DashScope text-embedding-v1 生成嵌入向量，按知识库类型隔离存储和检索。
 */
@Slf4j
@Service
@RequiredArgsConstructor
public class MultiKBRetrievalServiceImpl implements MultiKBRetrievalService, CommandLineRunner {

    @Override
    public void run(String... args) {
        initializeAllIndexes();
    }

    private static final String EMBEDDING_API_URL =
            "https://dashscope.aliyuncs.com/api/v1/services/embeddings/text-embedding/text-embedding";

    private final DashScopeConfig dashScopeConfig;
    private final RestClient dashscopeRestClient;
    private final KnowledgeBaseService knowledgeBaseService;
    private final ObjectMapper objectMapper;
    private final InMemoryVectorStore vectorStore;

    /** 知识条目缓存：ID → 实体 */
    private final Map<Long, KnowledgeBase> knowledgeCache = new ConcurrentHashMap<>();

    @Override
    public void initializeAllIndexes() {
        log.info("开始初始化多知识库向量索引...");

        // 加载所有已发布的知识条目
        List<KnowledgeBase> publishedList = knowledgeBaseService.list(
                new com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper<KnowledgeBase>()
                        .eq(KnowledgeBase::getStatus, 1)
        );

        if (publishedList.isEmpty()) {
            log.warn("未找到已发布的知识库条目，索引为空");
            return;
        }

        // 按 kbType 分组（优先用 kbType 字段，兼容旧 category 字段）
        Map<KnowledgeBaseType, List<KnowledgeBase>> grouped = publishedList.stream()
                .collect(Collectors.groupingBy(kb -> {
                    KnowledgeBaseType type = KnowledgeBaseType.fromKbType(kb.getKbType());
                    if (type == KnowledgeBaseType.FAQ && kb.getKbType() == null) {
                        // kbType 为空时回退到 category 映射
                        return KnowledgeBaseType.fromCategory(kb.getCategory());
                    }
                    return type;
                }));

        for (Map.Entry<KnowledgeBaseType, List<KnowledgeBase>> entry : grouped.entrySet()) {
            KnowledgeBaseType kbType = entry.getKey();
            List<KnowledgeBase> kbList = entry.getValue();

            // 缓存实体
            kbList.forEach(kb -> knowledgeCache.put(kb.getId(), kb));

            // 分批获取嵌入向量
            try {
                List<String> texts = kbList.stream()
                        .map(kb -> kb.getQuestion() + "\n" + kb.getAnswer())
                        .toList();
                int batchSize = 20;
                Map<Long, float[]> idVectorMap = new LinkedHashMap<>();
                for (int start = 0; start < texts.size(); start += batchSize) {
                    int end = Math.min(start + batchSize, texts.size());
                    List<String> batch = texts.subList(start, end);
                    List<float[]> batchEmbeddings = callEmbeddingApi(batch);
                    for (int j = 0; j < batch.size(); j++) {
                        idVectorMap.put(kbList.get(start + j).getId(), batchEmbeddings.get(j));
                    }
                    log.info("  [{}] 索引进度：{}/{}", kbType, end, texts.size());
                }
                vectorStore.index(kbType, idVectorMap);
            } catch (Exception e) {
                log.error("  [{}] 索引初始化失败", kbType, e);
            }
        }

        log.info("多知识库向量索引初始化完成，总计 {} 条", vectorStore.totalSize());
    }

    @Override
    public List<KnowledgeSource> retrieve(String query, KnowledgeBaseType kbType) {
        if (vectorStore.size(kbType) == 0) {
            log.warn("知识库 {} 索引为空，跳过检索", kbType);
            return Collections.emptyList();
        }

        try {
            float[] queryEmbedding = embedQuery(query);
            List<VectorStore.ScoredId> scoredIds = vectorStore.search(
                    kbType, queryEmbedding,
                    dashScopeConfig.getRag().getTopK(),
                    dashScopeConfig.getRag().getSimilarityThreshold()
            );

            List<KnowledgeSource> results = new ArrayList<>();
            for (VectorStore.ScoredId sid : scoredIds) {
                KnowledgeBase kb = knowledgeCache.get(sid.id());
                if (kb != null) {
                    results.add(KnowledgeSource.builder()
                            .id(kb.getId())
                            .question(kb.getQuestion())
                            .answer(kb.getAnswer())
                            .similarity(Math.round(sid.score() * 10000.0) / 10000.0)
                            .build());
                }
            }

            log.info("MultiKB RAG [{}}]: query='{}', 命中 {} 条",
                    kbType, query.length() > 50 ? query.substring(0, 50) + "..." : query, results.size());
            return results;

        } catch (Exception e) {
            log.error("MultiKB RAG 检索失败: type={}", kbType, e);
            return Collections.emptyList();
        }
    }

    @Override
    public float[] embedQuery(String query) {
        try {
            return callEmbeddingApi(Collections.singletonList(query)).get(0);
        } catch (Exception e) {
            throw new RuntimeException("Embedding API 调用失败", e);
        }
    }

    @Override
    public Map<KnowledgeBaseType, Integer> getIndexStats() {
        Map<KnowledgeBaseType, Integer> stats = new LinkedHashMap<>();
        for (KnowledgeBaseType type : KnowledgeBaseType.values()) {
            stats.put(type, vectorStore.size(type));
        }
        return stats;
    }

    @Override
    public int getTotalIndexedCount() {
        return vectorStore.totalSize();
    }

    @Override
    public void refreshAllIndexes() {
        vectorStore.clearAll();
        knowledgeCache.clear();
        initializeAllIndexes();
        log.info("多知识库索引已刷新");
    }

    // ==================== 私有方法 ====================

    private List<float[]> callEmbeddingApi(List<String> texts) {
        try {
            Map<String, Object> requestBody = new HashMap<>();
            requestBody.put("model", dashScopeConfig.getEmbeddingModel());
            requestBody.put("input", Map.of("texts", texts));

            String response = dashscopeRestClient.post()
                    .uri(EMBEDDING_API_URL)
                    .body(requestBody)
                    .retrieve()
                    .body(String.class);

            JsonNode root = objectMapper.readTree(response);
            JsonNode embeddings = root.path("output").path("embeddings");

            List<float[]> result = new ArrayList<>();
            for (JsonNode emb : embeddings) {
                JsonNode vecArr = emb.path("embedding");
                float[] vec = new float[vecArr.size()];
                for (int i = 0; i < vecArr.size(); i++) {
                    vec[i] = (float) vecArr.get(i).asDouble();
                }
                result.add(vec);
            }
            return result;

        } catch (Exception e) {
            throw new RuntimeException("DashScope Embedding API 调用失败: " + e.getMessage(), e);
        }
    }
}
