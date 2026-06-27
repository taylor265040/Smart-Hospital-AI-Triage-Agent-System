package com.studyback.hospitalservicesystem.agent.config;

import lombok.Data;

/**
 * Agent 配置属性 — 对应 application.yml 中 agent.* 配置块
 */
@Data
public class AgentProperties {

    /** 是否启用 Agent（false 时新端点返回 503） */
    private boolean enabled = true;

    /** DashScope API Key（复用已有配置） */
    private String dashscopeApiKey;

    /** Planner 使用的模型 */
    private String plannerModel = "qwen-turbo";

    /** Embedding 模型 */
    private String embeddingModel = "text-embedding-v1";

    /** 温度参数（0-1，越低越确定） */
    private double temperature = 0.3;

    /** 最大输出 tokens */
    private int maxTokens = 1500;

    /** ChatMemory 保留的最大轮数 */
    private int memoryWindowSize = 20;

    /** RAG 检索 Top-K */
    private int ragTopK = 5;

    /** RAG 相似度阈值 */
    private double ragSimilarityThreshold = 0.3;

    /** Planner 最大重试次数（JSON 解析失败时） */
    private int plannerMaxRetries = 2;
}
