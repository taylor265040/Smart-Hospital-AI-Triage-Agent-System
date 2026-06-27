package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.entity.Conversation;
import com.studyback.hospitalservicesystem.service.ConversationService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * 会话控制器
 */
@Tag(name = "会话管理", description = "会话（AI/人工）的CRUD操作")
@RestController
@RequestMapping("/api/conversation")
@RequiredArgsConstructor
public class ConversationController {

    private final ConversationService conversationService;

    @Operation(summary = "分页查询会话列表")
    @GetMapping("/list")
    public Result<PageResult<Conversation>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "10") Integer pageSize,
            @Parameter(description = "用户ID") @RequestParam(required = false) Long userId,
            @Parameter(description = "会话类型：AI/MANUAL") @RequestParam(required = false) String type,
            @Parameter(description = "会话状态：ACTIVE/CLOSED") @RequestParam(required = false) String status) {

        Page<Conversation> page = conversationService.pageWithCondition(pageNum, pageSize, userId, type, status);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询会话详情")
    @GetMapping("/{id}")
    public Result<Conversation> getById(@Parameter(description = "会话ID") @PathVariable Long id) {
        Conversation conversation = conversationService.getByIdOrNull(id);
        if (conversation == null) {
            return Result.notFound("会话不存在");
        }
        return Result.ok(conversation);
    }

    @Operation(summary = "新增会话")
    @PostMapping
    public Result<Conversation> create(@RequestBody Conversation conversation) {
        conversationService.save(conversation);
        return Result.ok("新增成功", conversation);
    }

    @Operation(summary = "更新会话（关闭/转人工）")
    @PutMapping
    public Result<Conversation> update(@RequestBody Conversation conversation) {
        if (conversation.getId() == null) {
            return Result.badRequest("会话ID不能为空");
        }
        Conversation exist = conversationService.getByIdOrNull(conversation.getId());
        if (exist == null) {
            return Result.notFound("会话不存在");
        }
        conversationService.updateById(conversation);
        return Result.ok("更新成功", conversationService.getByIdOrNull(conversation.getId()));
    }

    @Operation(summary = "删除会话")
    @DeleteMapping("/{id}")
    public Result<Void> delete(@Parameter(description = "会话ID") @PathVariable Long id) {
        boolean removed = conversationService.removeById(id);
        if (!removed) {
            return Result.notFound("会话不存在");
        }
        return Result.ok();
    }

    @Operation(summary = "批量删除会话")
    @DeleteMapping("/batch")
    public Result<Void> deleteBatch(@Parameter(description = "会话ID列表") @RequestBody List<Long> ids) {
        conversationService.removeByIds(ids);
        return Result.ok();
    }

}
