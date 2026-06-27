package com.studyback.hospitalservicesystem.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 登录 / 刷新令牌响应
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class LoginResponse {

    private String accessToken;
    private String refreshToken;

    /** access_token 过期时间戳（毫秒），前端据此判断何时刷新 */
    private long expiresAt;

    private Long userId;
    private String username;
    private String realName;
    private String role;
}
