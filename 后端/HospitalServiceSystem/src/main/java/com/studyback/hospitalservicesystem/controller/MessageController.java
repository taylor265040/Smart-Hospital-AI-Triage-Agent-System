package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.entity.Message;
import com.studyback.hospitalservicesystem.service.MessageService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * 消息控制器
 */
@Tag(name = "消息管理", description = "会话消息的CRUD操作")
@RestController
@RequestMapping("/api/message")
@RequiredArgsConstructor
public class MessageController {

    private final MessageService messageService;

    @Operation(summary = "分页查询消息列表")
    @GetMapping("/list")
    public Result<PageResult<Message>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "20") Integer pageSize,
            @Parameter(description = "会话ID") @RequestParam(required = false) Long conversationId,
            @Parameter(description = "发送者类型") @RequestParam(required = false) String senderType) {

        Page<Message> page = messageService.pageWithCondition(pageNum, pageSize, conversationId, senderType);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询会话的所有消息（按时间正序）")
    @GetMapping("/conversation/{conversationId}")
    public Result<List<Message>> getByConversationId(
            @Parameter(description = "会话ID") @PathVariable Long conversationId) {
        return Result.ok(messageService.listByConversationId(conversationId));
    }

    @Operation(summary = "查询消息详情")
    @GetMapping("/{id}")
    public Result<Message> getById(@Parameter(description = "消息ID") @PathVariable Long id) {
        Message message = messageService.getByIdOrNull(id);
        if (message == null) {
            return Result.notFound("消息不存在");
        }
        return Result.ok(message);
    }

    @Operation(summary = "新增消息")
    @PostMapping
    public Result<Message> create(@RequestBody Message message) {
        messageService.save(message);
        return Result.ok("新增成功", message);
    }

    @Operation(summary = "删除消息")
    @DeleteMapping("/{id}")
    public Result<Void> delete(@Parameter(description = "消息ID") @PathVariable Long id) {
        boolean removed = messageService.removeById(id);
        if (!removed) {
            return Result.notFound("消息不存在");
        }
        return Result.ok();
    }

    @Operation(summary = "批量删除消息")
    @DeleteMapping("/batch")
    public Result<Void> deleteBatch(@Parameter(description = "消息ID列表") @RequestBody List<Long> ids) {
        messageService.removeByIds(ids);
        return Result.ok();
    }

}
