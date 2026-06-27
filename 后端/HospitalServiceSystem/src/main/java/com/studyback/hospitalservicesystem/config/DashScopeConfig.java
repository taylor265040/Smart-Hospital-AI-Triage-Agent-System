package com.studyback.hospitalservicesystem.config;

import lombok.Data;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.client.JdkClientHttpRequestFactory;
import org.springframework.web.client.RestClient;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLParameters;
import java.net.http.HttpClient;
import java.security.NoSuchAlgorithmException;
import java.time.Duration;

/**
 * DashScope（阿里云百炼）配置
 */
@Data
@Configuration
@ConfigurationProperties(prefix = "dashscope")
public class DashScopeConfig {

    /** API Key */
    private String apiKey;

    /** 对话模型 */
    private String model = "qwen-turbo";

    /** 嵌入模型 */
    private String embeddingModel = "text-embedding-v1";

    /** RAG 配置 */
    private RagConfig rag = new RagConfig();

    @Data
    public static class RagConfig {
        /** 检索返回 Top-K 条结果 */
        private int topK = 5;

        /** 相似度阈值 */
        private double similarityThreshold = 0.3;
    }

    /**
     * DashScope 专用 RestClient（强制 TLS 1.2 + API Key Header）
     */
    @Bean
    public RestClient dashscopeRestClient() throws NoSuchAlgorithmException {
        SSLContext sslContext = SSLContext.getInstance("TLSv1.2");
        try {
            sslContext.init(null, null, null);
        } catch (Exception e) {
            throw new RuntimeException("SSL 初始化失败", e);
        }

        SSLParameters sslParams = new SSLParameters();
        sslParams.setProtocols(new String[]{"TLSv1.2"});

        HttpClient httpClient = HttpClient.newBuilder()
                .sslContext(sslContext)
                .sslParameters(sslParams)
                .connectTimeout(Duration.ofSeconds(10))
                .build();

        JdkClientHttpRequestFactory factory = new JdkClientHttpRequestFactory(httpClient);
        factory.setReadTimeout(Duration.ofSeconds(60));

        return RestClient.builder()
                .requestFactory(factory)
                .defaultHeader("Authorization", "Bearer " + apiKey)
                .defaultHeader("Content-Type", "application/json")
                .build();
    }

}
