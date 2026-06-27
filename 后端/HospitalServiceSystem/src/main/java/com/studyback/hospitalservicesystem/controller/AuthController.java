package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.config.JwtUtil;
import com.studyback.hospitalservicesystem.dto.LoginRequest;
import com.studyback.hospitalservicesystem.dto.LoginResponse;
import com.studyback.hospitalservicesystem.dto.RefreshRequest;
import com.studyback.hospitalservicesystem.entity.User;
import com.studyback.hospitalservicesystem.service.UserService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.util.DigestUtils;
import org.springframework.web.bind.annotation.*;

import java.nio.charset.StandardCharsets;

/**
 * 认证控制器 — JWT 登录 / 刷新令牌
 */
@Slf4j
@Tag(name = "认证管理", description = "JWT 登录认证与令牌刷新")
@RestController
@RequestMapping("/api/auth")
@RequiredArgsConstructor
public class AuthController {

    private final UserService userService;
    private final JwtUtil jwtUtil;

    @Operation(summary = "用户登录", description = "验证用户名密码，返回 access_token + refresh_token")
    @PostMapping("/login")
    public Result<LoginResponse> login(@Valid @RequestBody LoginRequest request) {
        // 1. 查找用户
        User user = userService.getOne(
                new LambdaQueryWrapper<User>()
                        .eq(User::getUsername, request.getUsername())
                        .last("LIMIT 1"),
                false
        );
        if (user == null) {
            return Result.fail("用户名或密码错误");
        }

        // 2. 校验密码（前端传明文，后端 MD5 后与数据库密文比对）
        String md5Password = DigestUtils.md5DigestAsHex(request.getPassword().getBytes(StandardCharsets.UTF_8));
        if (!md5Password.equals(user.getPassword())) {
            return Result.fail("用户名或密码错误");
        }

        // 3. 校验状态
        if (user.getStatus() != null && user.getStatus() != 1) {
            return Result.fail("账号已被禁用，请联系管理员");
        }

        // 4. 生成令牌
        long now = System.currentTimeMillis();
        String accessToken = jwtUtil.generateAccessToken(user.getId(), user.getUsername(), user.getRole());
        String refreshToken = jwtUtil.generateRefreshToken(user.getId());
        long expiresAt = now + 30 * 60 * 1000L; // 与 access-expiration 保持一致

        log.info("用户 {} ({}) 登录成功", user.getUsername(), user.getRealName());

        return Result.ok(LoginResponse.builder()
                .accessToken(accessToken)
                .refreshToken(refreshToken)
                .expiresAt(expiresAt)
                .userId(user.getId())
                .username(user.getUsername())
                .realName(user.getRealName())
                .role(user.getRole())
                .build());
    }

    @Operation(summary = "刷新令牌", description = "使用 refresh_token 换取新的 access_token")
    @PostMapping("/refresh")
    public Result<LoginResponse> refresh(@Valid @RequestBody RefreshRequest request) {
        // 1. 校验 refresh_token
        if (!jwtUtil.validateToken(request.getRefreshToken())) {
            return Result.fail("refresh_token 无效或已过期，请重新登录");
        }
        if (!jwtUtil.isRefreshToken(request.getRefreshToken())) {
            return Result.fail("非法的令牌类型");
        }

        // 2. 提取用户ID并查找用户
        Long userId = jwtUtil.getUserIdFromToken(request.getRefreshToken());
        User user = userService.getById(userId);
        if (user == null || user.getStatus() != 1) {
            return Result.fail("用户不存在或已被禁用");
        }

        // 3. 生成新的令牌对
        long now = System.currentTimeMillis();
        String newAccessToken = jwtUtil.generateAccessToken(user.getId(), user.getUsername(), user.getRole());
        String newRefreshToken = jwtUtil.generateRefreshToken(user.getId());
        long expiresAt = now + 30 * 60 * 1000L;

        log.info("用户 {} 刷新令牌成功", user.getUsername());

        return Result.ok(LoginResponse.builder()
                .accessToken(newAccessToken)
                .refreshToken(newRefreshToken)
                .expiresAt(expiresAt)
                .userId(user.getId())
                .username(user.getUsername())
                .realName(user.getRealName())
                .role(user.getRole())
                .build());
    }
}
