package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.entity.Rating;
import com.studyback.hospitalservicesystem.service.RatingService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * 满意度评价控制器
 */
@Tag(name = "满意度评价", description = "满意度评价的CRUD操作")
@RestController
@RequestMapping("/api/rating")
@RequiredArgsConstructor
public class RatingController {

    private final RatingService ratingService;

    @Operation(summary = "分页查询评价列表")
    @GetMapping("/list")
    public Result<PageResult<Rating>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "10") Integer pageSize,
            @Parameter(description = "用户ID") @RequestParam(required = false) Long userId,
            @Parameter(description = "最小评分") @RequestParam(required = false) Integer minScore) {

        Page<Rating> page = ratingService.pageWithCondition(pageNum, pageSize, userId, minScore);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询评价详情")
    @GetMapping("/{id}")
    public Result<Rating> getById(@Parameter(description = "评价ID") @PathVariable Long id) {
        Rating rating = ratingService.getByIdOrNull(id);
        if (rating == null) {
            return Result.notFound("评价不存在");
        }
        return Result.ok(rating);
    }

    @Operation(summary = "新增评价")
    @PostMapping
    public Result<Rating> create(@RequestBody Rating rating) {
        if (rating.getScore() == null || rating.getScore() < 1 || rating.getScore() > 5) {
            return Result.badRequest("评分必须在1-5之间");
        }
        ratingService.save(rating);
        return Result.ok("评价成功", rating);
    }

    @Operation(summary = "删除评价")
    @DeleteMapping("/{id}")
    public Result<Void> delete(@Parameter(description = "评价ID") @PathVariable Long id) {
        boolean removed = ratingService.removeById(id);
        if (!removed) {
            return Result.notFound("评价不存在");
        }
        return Result.ok();
    }

    @Operation(summary = "批量删除评价")
    @DeleteMapping("/batch")
    public Result<Void> deleteBatch(@Parameter(description = "评价ID列表") @RequestBody List<Long> ids) {
        ratingService.removeByIds(ids);
        return Result.ok();
    }

}
