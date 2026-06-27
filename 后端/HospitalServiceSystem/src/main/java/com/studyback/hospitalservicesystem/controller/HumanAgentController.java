package com.studyback.hospitalservicesystem.controller;

import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.dto.HumanServiceResult;
import com.studyback.hospitalservicesystem.service.HumanAgentService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

/**
 * 人工客服控制器
 */
@Tag(name = "人工客服", description = "排队、分配、接管、关闭会话")
@RestController
@RequestMapping("/api/cs")
@RequiredArgsConstructor
public class HumanAgentController {

    private final HumanAgentService humanAgentService;

    // ==================== 用户端 ====================

    @Operation(summary = "请求人工客服", description = "用户点击\"转人工\"后调用，进入排队队列")
    @PostMapping("/request-human")
    public Result<HumanServiceResult> requestHuman(
            @Parameter(description = "用户ID") @RequestParam Long userId,
            @Parameter(description = "请求原因") @RequestParam(required = false) String reason) {
        HumanServiceResult result = humanAgentService.requestHumanService(userId, reason);
        return Result.ok(result);
    }

    @Operation(summary = "查询排队状态", description = "用户查询当前排队位置")
    @GetMapping("/queue-status")
    public Result<HumanServiceResult> queueStatus(
            @Parameter(description = "用户ID") @RequestParam Long userId) {
        return Result.ok(humanAgentService.checkQueueStatus(userId));
    }

    @Operation(summary = "取消排队", description = "用户取消等待")
    @PostMapping("/cancel-queue")
    public Result<Void> cancelQueue(
            @Parameter(description = "用户ID") @RequestParam Long userId) {
        humanAgentService.cancelQueue(userId);
        return Result.ok();
    }

    // ==================== 客服端 ====================

    @Operation(summary = "客服上线")
    @PostMapping("/staff-online")
    public Result<Void> staffOnline(
            @Parameter(description = "客服用户ID") @RequestParam Long csStaffId) {
        humanAgentService.staffOnline(csStaffId);
        return Result.ok();
    }

    @Operation(summary = "客服下线")
    @PostMapping("/staff-offline")
    public Result<Void> staffOffline(
            @Parameter(description = "客服用户ID") @RequestParam Long csStaffId) {
        humanAgentService.staffOffline(csStaffId);
        return Result.ok();
    }

    @Operation(summary = "排队列表（客服端）", description = "查看当前所有等待人工服务的用户")
    @GetMapping("/pending-queue")
    public Result<List<Map<String, Object>>> pendingQueue() {
        return Result.ok(humanAgentService.getPendingQueue());
    }

    @Operation(summary = "接受用户（客服端）", description = "客服接受某个排队用户，开始一对一对话")
    @PostMapping("/accept")
    public Result<HumanServiceResult> accept(
            @Parameter(description = "客服ID") @RequestParam Long csStaffId,
            @Parameter(description = "排队会话ID") @RequestParam Long conversationId) {
        HumanServiceResult result = humanAgentService.acceptUser(csStaffId, conversationId);
        return result.isSuccess() ? Result.ok(result) : Result.fail(result.getMessage());
    }

    @Operation(summary = "关闭会话（客服端）", description = "客服结束人工服务")
    @PostMapping("/close-session")
    public Result<Void> closeSession(
            @Parameter(description = "客服ID") @RequestParam Long csStaffId,
            @Parameter(description = "会话ID") @RequestParam Long conversationId) {
        try {
            humanAgentService.closeSession(csStaffId, conversationId);
            return Result.ok();
        } catch (IllegalArgumentException e) {
            return Result.fail(e.getMessage());
        }
    }

    @Operation(summary = "在线客服列表", description = "获取当前在线客服及其服务负载")
    @GetMapping("/online-staff")
    public Result<List<Map<String, Object>>> onlineStaff() {
        return Result.ok(humanAgentService.getOnlineStaff());
    }

}
