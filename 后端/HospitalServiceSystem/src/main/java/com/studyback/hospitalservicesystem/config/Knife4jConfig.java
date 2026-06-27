package com.studyback.hospitalservicesystem.config;

import io.swagger.v3.oas.models.OpenAPI;
import io.swagger.v3.oas.models.info.Contact;
import io.swagger.v3.oas.models.info.Info;
import io.swagger.v3.oas.models.info.License;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

/**
 * Knife4j (Swagger) API 文档配置
 */
@Configuration
public class Knife4jConfig {

    @Bean
    public OpenAPI customOpenAPI() {
        return new OpenAPI()
                .info(new Info()
                        .title("医院智慧客服系统 API 文档")
                        .version("1.0.0")
                        .description("医院智慧客服系统（Hospital Smart Customer Service System）后端接口文档")
                        .contact(new Contact()
                                .name("开发团队")
                                .email("dev@hospital.com"))
                        .license(new License()
                                .name("私有授权")
                                .url("https://www.hospital.com")));
    }

}
