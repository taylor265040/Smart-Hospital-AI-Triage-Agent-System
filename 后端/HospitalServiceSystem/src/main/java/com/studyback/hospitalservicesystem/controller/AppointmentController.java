package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.dto.AppointmentRequest;
import com.studyback.hospitalservicesystem.dto.TimeSlotVO;
import com.studyback.hospitalservicesystem.entity.Appointment;
import com.studyback.hospitalservicesystem.entity.Department;
import com.studyback.hospitalservicesystem.entity.Doctor;
import com.studyback.hospitalservicesystem.service.AppointmentService;
import com.studyback.hospitalservicesystem.service.DepartmentService;
import com.studyback.hospitalservicesystem.service.DoctorService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDate;
import java.util.List;

/**
 * 预约挂号控制器
 */
@Tag(name = "预约挂号", description = "科室/医生查询 + 时段选择 + 预约/取消")
@RestController
@RequestMapping("/api/appointment")
@RequiredArgsConstructor
public class AppointmentController {

    private final AppointmentService appointmentService;
    private final DepartmentService departmentService;
    private final DoctorService doctorService;

    // ==================== 查询 ====================

    @Operation(summary = "获取可预约科室列表")
    @GetMapping("/departments")
    public Result<List<Department>> departments() {
        return Result.ok(departmentService.listEnabled());
    }

    @Operation(summary = "获取某科室医生列表")
    @GetMapping("/doctors")
    public Result<List<Doctor>> doctors(
            @Parameter(description = "科室ID") @RequestParam Long departmentId) {
        List<Doctor> doctors = doctorService.list(
                new com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper<Doctor>()
                        .eq(Doctor::getDepartmentId, departmentId)
                        .eq(Doctor::getStatus, 1)
        );
        return Result.ok(doctors);
    }

    @Operation(summary = "获取某医生某日可用时段")
    @GetMapping("/slots")
    public Result<List<TimeSlotVO>> slots(
            @Parameter(description = "医生ID") @RequestParam Long doctorId,
            @Parameter(description = "日期 yyyy-MM-dd") @RequestParam String date) {
        LocalDate localDate = LocalDate.parse(date);
        return Result.ok(appointmentService.getAvailableSlots(doctorId, localDate));
    }

    @Operation(summary = "分页查询预约记录")
    @GetMapping("/list")
    public Result<PageResult<Appointment>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "10") Integer pageSize,
            @Parameter(description = "用户ID") @RequestParam(required = false) Long userId,
            @Parameter(description = "医生ID") @RequestParam(required = false) Long doctorId,
            @Parameter(description = "状态") @RequestParam(required = false) String status) {
        Page<Appointment> page = appointmentService.pageWithCondition(pageNum, pageSize, userId, doctorId, status);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询预约详情")
    @GetMapping("/{id}")
    public Result<Appointment> getById(@Parameter(description = "预约ID") @PathVariable Long id) {
        Appointment appointment = appointmentService.getByIdOrNull(id);
        if (appointment == null) {
            return Result.notFound("预约不存在");
        }
        return Result.ok(appointment);
    }

    // ==================== 操作 ====================

    @Operation(summary = "提交预约", description = "选择科室→医生→时段后提交预约")
    @PostMapping("/book")
    public Result<Appointment> book(@Valid @RequestBody AppointmentRequest request) {
        try {
            Appointment appointment = appointmentService.book(request);
            return Result.ok("预约成功", appointment);
        } catch (IllegalArgumentException e) {
            return Result.fail(e.getMessage());
        }
    }

    @Operation(summary = "取消预约")
    @PutMapping("/{id}/cancel")
    public Result<Void> cancel(
            @Parameter(description = "预约ID") @PathVariable Long id,
            @Parameter(description = "用户ID（校验权限）") @RequestParam Long userId) {
        try {
            appointmentService.cancel(id, userId);
            return Result.ok();
        } catch (IllegalArgumentException e) {
            return Result.fail(e.getMessage());
        }
    }

    @Operation(summary = "医生确认预约")
    @PutMapping("/{id}/confirm")
    public Result<Appointment> confirm(
            @Parameter(description = "预约ID") @PathVariable Long id,
            @Parameter(description = "医生ID（校验权限）") @RequestParam Long doctorId) {
        try {
            Appointment appointment = appointmentService.confirm(id, doctorId);
            return Result.ok("确认成功", appointment);
        } catch (IllegalArgumentException e) {
            return Result.fail(e.getMessage());
        }
    }

}
