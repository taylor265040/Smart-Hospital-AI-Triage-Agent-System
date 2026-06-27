package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.entity.KnowledgeBase;
import com.studyback.hospitalservicesystem.service.KnowledgeBaseService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * 知识库控制器
 */
@Tag(name = "知识库管理", description = "知识库FAQ的CRUD操作")
@RestController
@RequestMapping("/api/knowledge-base")
@RequiredArgsConstructor
public class KnowledgeBaseController {

    private final KnowledgeBaseService knowledgeBaseService;

    @Operation(summary = "分页查询知识库列表")
    @GetMapping("/list")
    public Result<PageResult<KnowledgeBase>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "10") Integer pageSize,
            @Parameter(description = "问题关键词") @RequestParam(required = false) String keyword,
            @Parameter(description = "分类") @RequestParam(required = false) String category,
            @Parameter(description = "状态") @RequestParam(required = false) Integer status) {

        Page<KnowledgeBase> page = knowledgeBaseService.pageWithCondition(pageNum, pageSize, keyword, category, status);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询知识库详情")
    @GetMapping("/{id}")
    public Result<KnowledgeBase> getById(@Parameter(description = "知识库ID") @PathVariable Long id) {
        KnowledgeBase kb = knowledgeBaseService.getByIdAndIncrementView(id);
        if (kb == null) {
            return Result.notFound("知识库条目不存在");
        }
        return Result.ok(kb);
    }

    @Operation(summary = "新增知识库条目")
    @PostMapping
    public Result<KnowledgeBase> create(@RequestBody KnowledgeBase knowledgeBase) {
        knowledgeBaseService.save(knowledgeBase);
        return Result.ok("新增成功", knowledgeBase);
    }

    @Operation(summary = "更新知识库条目")
    @PutMapping
    public Result<KnowledgeBase> update(@RequestBody KnowledgeBase knowledgeBase) {
        if (knowledgeBase.getId() == null) {
            return Result.badRequest("知识库ID不能为空");
        }
        KnowledgeBase exist = knowledgeBaseService.getByIdOrNull(knowledgeBase.getId());
        if (exist == null) {
            return Result.notFound("知识库条目不存在");
        }
        knowledgeBaseService.updateById(knowledgeBase);
        return Result.ok("更新成功", knowledgeBaseService.getByIdOrNull(knowledgeBase.getId()));
    }

    @Operation(summary = "删除知识库条目")
    @DeleteMapping("/{id}")
    public Result<Void> delete(@Parameter(description = "知识库ID") @PathVariable Long id) {
        boolean removed = knowledgeBaseService.removeById(id);
        if (!removed) {
            return Result.notFound("知识库条目不存在");
        }
        return Result.ok();
    }

    @Operation(summary = "批量删除知识库条目")
    @DeleteMapping("/batch")
    public Result<Void> deleteBatch(@Parameter(description = "知识库ID列表") @RequestBody List<Long> ids) {
        knowledgeBaseService.removeByIds(ids);
        return Result.ok();
    }

}
