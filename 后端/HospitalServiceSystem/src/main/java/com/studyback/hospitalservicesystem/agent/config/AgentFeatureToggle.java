package com.studyback.hospitalservicesystem.agent.config;

import lombok.Data;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

/**
 * Agent 功能开关 — 控制新版 Agent 端点的启用/禁用
 */
@Data
@Configuration
public class AgentFeatureToggle {

    @Bean
    @ConfigurationProperties(prefix = "agent.feature")
    public FeatureProperties featureProperties() {
        return new FeatureProperties();
    }

    @Data
    public static class FeatureProperties {
        /** Agent 是否启用 */
        private boolean enabled = true;
    }
}
