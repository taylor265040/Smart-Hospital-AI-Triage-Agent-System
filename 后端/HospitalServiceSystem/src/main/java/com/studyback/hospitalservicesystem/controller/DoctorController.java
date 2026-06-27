package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.entity.Doctor;
import com.studyback.hospitalservicesystem.service.DoctorService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * 医生控制器
 */
@Tag(name = "医生管理", description = "医生的CRUD操作")
@RestController
@RequestMapping("/api/doctor")
@RequiredArgsConstructor
public class DoctorController {

    private final DoctorService doctorService;

    @Operation(summary = "分页查询医生列表")
    @GetMapping("/list")
    public Result<PageResult<Doctor>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "10") Integer pageSize,
            @Parameter(description = "医生姓名（模糊搜索）") @RequestParam(required = false) String keyword,
            @Parameter(description = "科室ID") @RequestParam(required = false) Long departmentId) {

        Page<Doctor> page = doctorService.pageWithCondition(pageNum, pageSize, keyword, departmentId);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询医生详情")
    @GetMapping("/{id}")
    public Result<Doctor> getById(@Parameter(description = "医生ID") @PathVariable Long id) {
        Doctor doctor = doctorService.getByIdOrNull(id);
        if (doctor == null) {
            return Result.notFound("医生不存在");
        }
        return Result.ok(doctor);
    }

    @Operation(summary = "新增医生")
    @PostMapping
    public Result<Doctor> create(@RequestBody Doctor doctor) {
        doctorService.save(doctor);
        return Result.ok("新增成功", doctor);
    }

    @Operation(summary = "更新医生")
    @PutMapping
    public Result<Doctor> update(@RequestBody Doctor doctor) {
        if (doctor.getId() == null) {
            return Result.badRequest("医生ID不能为空");
        }
        Doctor exist = doctorService.getByIdOrNull(doctor.getId());
        if (exist == null) {
            return Result.notFound("医生不存在");
        }
        doctorService.updateById(doctor);
        return Result.ok("更新成功", doctorService.getByIdOrNull(doctor.getId()));
    }

    @Operation(summary = "删除医生")
    @DeleteMapping("/{id}")
    public Result<Void> delete(@Parameter(description = "医生ID") @PathVariable Long id) {
        boolean removed = doctorService.removeById(id);
        if (!removed) {
            return Result.notFound("医生不存在");
        }
        return Result.ok();
    }

    @Operation(summary = "批量删除医生")
    @DeleteMapping("/batch")
    public Result<Void> deleteBatch(@Parameter(description = "医生ID列表") @RequestBody List<Long> ids) {
        doctorService.removeByIds(ids);
        return Result.ok();
    }

}
