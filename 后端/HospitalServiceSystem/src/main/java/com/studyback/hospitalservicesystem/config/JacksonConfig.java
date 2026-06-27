package com.studyback.hospitalservicesystem.config;

import com.fasterxml.jackson.databind.ser.std.ToStringSerializer;
import org.springframework.boot.autoconfigure.jackson.Jackson2ObjectMapperBuilderCustomizer;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

/**
 * Jackson 序列化配置
 * 核心：将 Long 类型序列化为字符串，解决 JavaScript Number 精度丢失问题
 * JS 的 Number 只能安全表示 2^53-1 以内的整数，而 MyBatis-Plus 雪花算法生成的 ID 为 19 位 Long
 */
@Configuration
public class JacksonConfig {

    @Bean
    public Jackson2ObjectMapperBuilderCustomizer longToStringCustomizer() {
        return builder -> {
            // Long 类型全局序列化为字符串（雪花 ID 安全传输）
            builder.serializerByType(Long.class, ToStringSerializer.instance);
            // long 基本类型也序列化为字符串（统一行为）
            builder.serializerByType(long.class, ToStringSerializer.instance);
        };
    }
}
