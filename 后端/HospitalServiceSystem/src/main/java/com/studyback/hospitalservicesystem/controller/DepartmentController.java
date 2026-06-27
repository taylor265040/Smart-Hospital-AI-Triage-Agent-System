package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.entity.Department;
import com.studyback.hospitalservicesystem.service.DepartmentService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * 科室控制器
 */
@Tag(name = "科室管理", description = "科室的CRUD操作")
@RestController
@RequestMapping("/api/department")
@RequiredArgsConstructor
public class DepartmentController {

    private final DepartmentService departmentService;

    @Operation(summary = "分页查询科室列表")
    @GetMapping("/list")
    public Result<PageResult<Department>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "10") Integer pageSize,
            @Parameter(description = "科室名称（模糊搜索）") @RequestParam(required = false) String keyword) {

        Page<Department> page = departmentService.pageWithCondition(pageNum, pageSize, keyword);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询所有启用的科室")
    @GetMapping("/all")
    public Result<List<Department>> all() {
        return Result.ok(departmentService.listEnabled());
    }

    @Operation(summary = "查询科室详情")
    @GetMapping("/{id}")
    public Result<Department> getById(@Parameter(description = "科室ID") @PathVariable Long id) {
        Department department = departmentService.getByIdOrNull(id);
        if (department == null) {
            return Result.notFound("科室不存在");
        }
        return Result.ok(department);
    }

    @Operation(summary = "新增科室")
    @PostMapping
    public Result<Department> create(@RequestBody Department department) {
        if (departmentService.existsByName(department.getName(), null)) {
            return Result.fail("科室名称已存在");
        }
        departmentService.save(department);
        return Result.ok("新增成功", department);
    }

    @Operation(summary = "更新科室")
    @PutMapping
    public Result<Department> update(@RequestBody Department department) {
        if (department.getId() == null) {
            return Result.badRequest("科室ID不能为空");
        }
        Department exist = departmentService.getByIdOrNull(department.getId());
        if (exist == null) {
            return Result.notFound("科室不存在");
        }
        if (departmentService.existsByName(department.getName(), department.getId())) {
            return Result.fail("科室名称已存在");
        }
        departmentService.updateById(department);
        return Result.ok("更新成功", departmentService.getByIdOrNull(department.getId()));
    }

    @Operation(summary = "删除科室")
    @DeleteMapping("/{id}")
    public Result<Void> delete(@Parameter(description = "科室ID") @PathVariable Long id) {
        boolean removed = departmentService.removeById(id);
        if (!removed) {
            return Result.notFound("科室不存在");
        }
        return Result.ok();
    }

    @Operation(summary = "批量删除科室")
    @DeleteMapping("/batch")
    public Result<Void> deleteBatch(@Parameter(description = "科室ID列表") @RequestBody List<Long> ids) {
        departmentService.removeByIds(ids);
        return Result.ok();
    }

}
