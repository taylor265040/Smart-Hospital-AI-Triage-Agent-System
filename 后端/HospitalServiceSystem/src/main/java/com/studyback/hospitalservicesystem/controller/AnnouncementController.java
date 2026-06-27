package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.entity.Announcement;
import com.studyback.hospitalservicesystem.service.AnnouncementService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.util.List;

/**
 * 公告控制器
 */
@Tag(name = "公告管理", description = "医院公告的CRUD操作")
@RestController
@RequestMapping("/api/announcement")
@RequiredArgsConstructor
public class AnnouncementController {

    private final AnnouncementService announcementService;

    @Operation(summary = "分页查询公告列表")
    @GetMapping("/list")
    public Result<PageResult<Announcement>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "10") Integer pageSize,
            @Parameter(description = "公告标题") @RequestParam(required = false) String keyword,
            @Parameter(description = "公告类型") @RequestParam(required = false) String type,
            @Parameter(description = "状态") @RequestParam(required = false) Integer status) {

        Page<Announcement> page = announcementService.pageWithCondition(pageNum, pageSize, keyword, type, status);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询已发布的公告（患者端）")
    @GetMapping("/published")
    public Result<List<Announcement>> published() {
        return Result.ok(announcementService.listPublished());
    }

    @Operation(summary = "查询公告详情")
    @GetMapping("/{id}")
    public Result<Announcement> getById(@Parameter(description = "公告ID") @PathVariable Long id) {
        Announcement announcement = announcementService.getByIdOrNull(id);
        if (announcement == null) {
            return Result.notFound("公告不存在");
        }
        return Result.ok(announcement);
    }

    @Operation(summary = "新增公告")
    @PostMapping
    public Result<Announcement> create(@RequestBody Announcement announcement) {
        if (announcement.getStatus() != null && announcement.getStatus() == 1) {
            announcement.setPublishTime(LocalDateTime.now());
        }
        announcementService.save(announcement);
        return Result.ok("新增成功", announcement);
    }

    @Operation(summary = "更新公告")
    @PutMapping
    public Result<Announcement> update(@RequestBody Announcement announcement) {
        if (announcement.getId() == null) {
            return Result.badRequest("公告ID不能为空");
        }
        Announcement exist = announcementService.getByIdOrNull(announcement.getId());
        if (exist == null) {
            return Result.notFound("公告不存在");
        }
        // 如果状态改为已发布，设置发布时间
        if (announcement.getStatus() != null && announcement.getStatus() == 1
                && announcement.getPublishTime() == null) {
            announcement.setPublishTime(LocalDateTime.now());
        }
        announcementService.updateById(announcement);
        return Result.ok("更新成功", announcementService.getByIdOrNull(announcement.getId()));
    }

    @Operation(summary = "发布公告")
    @PutMapping("/publish/{id}")
    public Result<Announcement> publish(@Parameter(description = "公告ID") @PathVariable Long id) {
        Announcement announcement = announcementService.getByIdOrNull(id);
        if (announcement == null) {
            return Result.notFound("公告不存在");
        }
        announcementService.publish(id);
        return Result.ok("发布成功", announcementService.getByIdOrNull(id));
    }

    @Operation(summary = "删除公告")
    @DeleteMapping("/{id}")
    public Result<Void> delete(@Parameter(description = "公告ID") @PathVariable Long id) {
        boolean removed = announcementService.removeById(id);
        if (!removed) {
            return Result.notFound("公告不存在");
        }
        return Result.ok();
    }

    @Operation(summary = "批量删除公告")
    @DeleteMapping("/batch")
    public Result<Void> deleteBatch(@Parameter(description = "公告ID列表") @RequestBody List<Long> ids) {
        announcementService.removeByIds(ids);
        return Result.ok();
    }

}
