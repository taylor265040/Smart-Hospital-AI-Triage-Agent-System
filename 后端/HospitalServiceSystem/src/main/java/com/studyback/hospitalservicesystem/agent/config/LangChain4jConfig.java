package com.studyback.hospitalservicesystem.agent.config;

import dev.langchain4j.community.model.dashscope.QwenChatModel;
import dev.langchain4j.community.model.dashscope.QwenEmbeddingModel;
import dev.langchain4j.model.chat.ChatLanguageModel;
import dev.langchain4j.model.embedding.EmbeddingModel;
import lombok.extern.slf4j.Slf4j;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

/**
 * LangChain4j Agent 配置
 * <p>
 * 使用 LangChain4j Community DashScope 模块封装 Qwen 聊天模型和 Embedding 模型，
 * 替代旧的手工 RestClient 调用方式。
 */
@Slf4j
@Configuration
public class LangChain4jConfig {

    /**
     * LangChain4j ChatLanguageModel — 用于 Planner 和 Generator
     */
    @Bean
    @ConditionalOnProperty(prefix = "agent", name = "enabled", havingValue = "true", matchIfMissing = true)
    public ChatLanguageModel chatLanguageModel(AgentProperties agentProperties) {
        log.info("初始化 LangChain4j QwenChatModel: model={}, temperature={}",
                agentProperties.getPlannerModel(), agentProperties.getTemperature());
        return QwenChatModel.builder()
                .apiKey(agentProperties.getDashscopeApiKey())
                .modelName(agentProperties.getPlannerModel())
                .temperature((float) agentProperties.getTemperature())
                .maxTokens(agentProperties.getMaxTokens())
                .build();
    }

    /**
     * LangChain4j EmbeddingModel — 用于 RAG 向量检索
     */
    @Bean
    @ConditionalOnProperty(prefix = "agent", name = "enabled", havingValue = "true", matchIfMissing = true)
    public EmbeddingModel embeddingModel(AgentProperties agentProperties) {
        log.info("初始化 LangChain4j QwenEmbeddingModel: model={}",
                agentProperties.getEmbeddingModel());
        return QwenEmbeddingModel.builder()
                .apiKey(agentProperties.getDashscopeApiKey())
                .modelName(agentProperties.getEmbeddingModel())
                .build();
    }

    /**
     * Agent 配置属性
     */
    @Bean
    @ConfigurationProperties(prefix = "agent")
    public AgentProperties agentProperties() {
        return new AgentProperties();
    }
}
