package com.studyback.hospitalservicesystem.config;

import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;

/**
 * JWT 认证过滤器
 * 拦截所有 /api/** 请求（除 /api/auth/** 登录接口），校验 Authorization: Bearer <token>
 */
@Slf4j
@Component
@RequiredArgsConstructor
public class JwtAuthFilter extends OncePerRequestFilter {

    private final JwtUtil jwtUtil;

    /** 无需认证的路径前缀 */
    private static final String[] PUBLIC_PATHS = {
            "/api/auth/login",
            "/api/auth/refresh"
    };

    @Override
    protected void doFilterInternal(HttpServletRequest request,
                                    HttpServletResponse response,
                                    FilterChain filterChain) throws ServletException, IOException {

        String path = request.getRequestURI();

        // 公开路径放行
        for (String publicPath : PUBLIC_PATHS) {
            if (path.startsWith(publicPath)) {
                filterChain.doFilter(request, response);
                return;
            }
        }

        // Knife4j / Swagger 文档放行
        if (path.startsWith("/doc.html") || path.startsWith("/swagger-ui") ||
                path.startsWith("/v3/api-docs") || path.startsWith("/webjars")) {
            filterChain.doFilter(request, response);
            return;
        }

        // Druid 监控放行
        if (path.startsWith("/druid")) {
            filterChain.doFilter(request, response);
            return;
        }

        // 提取 Authorization header
        String authHeader = request.getHeader("Authorization");
        if (authHeader == null || !authHeader.startsWith("Bearer ")) {
            sendUnauthorized(response, "缺少认证令牌");
            return;
        }

        String token = authHeader.substring(7);
        if (!jwtUtil.validateToken(token)) {
            sendUnauthorized(response, "令牌无效或已过期");
            return;
        }

        // 将用户ID存入请求属性，方便后续使用
        try {
            Long userId = jwtUtil.getUserIdFromToken(token);
            request.setAttribute("userId", userId);
        } catch (Exception e) {
            sendUnauthorized(response, "令牌解析失败");
            return;
        }

        filterChain.doFilter(request, response);
    }

    private void sendUnauthorized(HttpServletResponse response, String message) throws IOException {
        response.setStatus(HttpServletResponse.SC_UNAUTHORIZED);
        response.setContentType("application/json;charset=UTF-8");
        response.getWriter().write(String.format(
                "{\"code\":401,\"message\":\"%s\",\"data\":null,\"timestamp\":%d}",
                message, System.currentTimeMillis()
        ));
    }
}
