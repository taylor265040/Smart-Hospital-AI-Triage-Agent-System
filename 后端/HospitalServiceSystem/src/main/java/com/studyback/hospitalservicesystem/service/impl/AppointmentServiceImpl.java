package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.studyback.hospitalservicesystem.dto.AppointmentRequest;
import com.studyback.hospitalservicesystem.dto.TimeSlotVO;
import com.studyback.hospitalservicesystem.entity.Appointment;
import com.studyback.hospitalservicesystem.mapper.AppointmentMapper;
import com.studyback.hospitalservicesystem.service.AppointmentService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.time.LocalDate;
import java.time.LocalTime;
import java.util.*;
import java.util.stream.Collectors;

/**
 * 预约挂号服务实现
 * <p>
 * 工作时段：上午 8:00-12:00 | 下午 14:00-18:00，每小时一个时段。
 */
@Slf4j
@Service
public class AppointmentServiceImpl extends ServiceImpl<AppointmentMapper, Appointment> implements AppointmentService {

    /** 上午工作时段 */
    private static final LocalTime MORNING_START = LocalTime.of(8, 0);
    private static final LocalTime MORNING_END = LocalTime.of(12, 0);

    /** 下午工作时段 */
    private static final LocalTime AFTERNOON_START = LocalTime.of(14, 0);
    private static final LocalTime AFTERNOON_END = LocalTime.of(18, 0);

    @Override
    public Page<Appointment> pageWithCondition(Integer pageNum, Integer pageSize, Long userId, Long doctorId, String status) {
        Page<Appointment> page = new Page<>(pageNum, pageSize);
        LambdaQueryWrapper<Appointment> wrapper = new LambdaQueryWrapper<>();

        if (userId != null) {
            wrapper.eq(Appointment::getUserId, userId);
        }
        if (doctorId != null) {
            wrapper.eq(Appointment::getDoctorId, doctorId);
        }
        if (status != null) {
            wrapper.eq(Appointment::getStatus, status);
        }
        wrapper.orderByDesc(Appointment::getAppointmentDate)
                .orderByAsc(Appointment::getStartTime);

        return this.page(page, wrapper);
    }

    @Override
    public List<TimeSlotVO> getAvailableSlots(Long doctorId, LocalDate date) {
        // 1. 生成全天所有时段（上午8个 + 下午4个）
        List<TimeSlotVO> allSlots = new ArrayList<>();
        generateSlots(allSlots, MORNING_START, MORNING_END, date, "上午");
        generateSlots(allSlots, AFTERNOON_START, AFTERNOON_END, date, "下午");

        // 2. 过滤已过去的时段（今天）
        if (date.equals(LocalDate.now())) {
            LocalTime now = LocalTime.now();
            allSlots.removeIf(slot -> slot.getStartTime().isBefore(now.plusMinutes(30)));
        }

        // 3. 查询当天已有预约
        List<Appointment> booked = this.list(
                new LambdaQueryWrapper<Appointment>()
                        .eq(Appointment::getDoctorId, doctorId)
                        .eq(Appointment::getAppointmentDate, date)
                        .in(Appointment::getStatus, "PENDING", "CONFIRMED")
        );
        Set<LocalTime> bookedStarts = booked.stream()
                .map(Appointment::getStartTime)
                .collect(Collectors.toSet());

        // 4. 标记可用性
        for (TimeSlotVO slot : allSlots) {
            slot.setAvailable(!bookedStarts.contains(slot.getStartTime()));
        }

        return allSlots;
    }

    /** 单个用户最大有效预约数 */
    private static final int MAX_ACTIVE_APPOINTMENTS = 3;

    @Override
    public Appointment book(AppointmentRequest request) {
        // 1. 校验时段有效性
        LocalTime start = request.getStartTime();
        if (start.getMinute() != 0) {
            throw new IllegalArgumentException("预约时间必须为整点");
        }
        if (!isWorkingHour(start)) {
            throw new IllegalArgumentException("预约时间必须在工作时段内（8:00-12:00 或 14:00-18:00）");
        }

        // 2. 检查用户有效预约数量上限（PENDING + CONFIRMED ≤ 3）
        long userActiveCount = this.count(new LambdaQueryWrapper<Appointment>()
                .eq(Appointment::getUserId, request.getUserId())
                .in(Appointment::getStatus, "PENDING", "CONFIRMED"));
        if (userActiveCount >= MAX_ACTIVE_APPOINTMENTS) {
            throw new IllegalArgumentException("您已有 " + MAX_ACTIVE_APPOINTMENTS + " 笔有效预约，请先取消后再预约");
        }

        // 3. 检查用户自身时段冲突（同一用户不能在同一时段预约不同医生）
        long userSlotConflict = this.count(new LambdaQueryWrapper<Appointment>()
                .eq(Appointment::getUserId, request.getUserId())
                .eq(Appointment::getAppointmentDate, request.getAppointmentDate())
                .eq(Appointment::getStartTime, start)
                .in(Appointment::getStatus, "PENDING", "CONFIRMED"));
        if (userSlotConflict > 0) {
            throw new IllegalArgumentException("您在该时段已有预约，请选择其他时段");
        }

        // 4. 检查医生时段冲突
        long doctorConflict = this.count(new LambdaQueryWrapper<Appointment>()
                .eq(Appointment::getDoctorId, request.getDoctorId())
                .eq(Appointment::getAppointmentDate, request.getAppointmentDate())
                .eq(Appointment::getStartTime, start)
                .in(Appointment::getStatus, "PENDING", "CONFIRMED"));
        if (doctorConflict > 0) {
            throw new IllegalArgumentException("该医生该时段已被预约，请选择其他时段");
        }

        // 5. 创建预约
        Appointment appointment = new Appointment();
        appointment.setUserId(request.getUserId());
        appointment.setDoctorId(request.getDoctorId());
        appointment.setDepartmentId(request.getDepartmentId());
        appointment.setAppointmentDate(request.getAppointmentDate());
        appointment.setStartTime(start);
        appointment.setEndTime(start.plusHours(1));
        appointment.setStatus("PENDING");
        appointment.setRemark(request.getRemark());

        this.save(appointment);
        log.info("预约成功：用户{} → 医生{} | {} {}:00-{}:00",
                request.getUserId(), request.getDoctorId(),
                request.getAppointmentDate(), start, start.plusHours(1));

        return appointment;
    }

    @Override
    public void cancel(Long appointmentId, Long userId) {
        Appointment appointment = this.getById(appointmentId);
        if (appointment == null) {
            throw new IllegalArgumentException("预约不存在");
        }
        if (!appointment.getUserId().equals(userId)) {
            throw new IllegalArgumentException("无权取消他人预约");
        }
        if ("CANCELLED".equals(appointment.getStatus())) {
            throw new IllegalArgumentException("预约已取消，无需重复操作");
        }
        appointment.setStatus("CANCELLED");
        this.updateById(appointment);
        log.info("预约已取消：id={}, 用户={}", appointmentId, userId);
    }

    @Override
    public Appointment confirm(Long appointmentId, Long doctorId) {
        Appointment appointment = this.getById(appointmentId);
        if (appointment == null) {
            throw new IllegalArgumentException("预约不存在");
        }
        if (!appointment.getDoctorId().equals(doctorId)) {
            throw new IllegalArgumentException("无权确认他人预约");
        }
        if ("CANCELLED".equals(appointment.getStatus())) {
            throw new IllegalArgumentException("预约已取消，无法确认");
        }
        if ("CONFIRMED".equals(appointment.getStatus())) {
            throw new IllegalArgumentException("预约已确认，无需重复操作");
        }
        appointment.setStatus("CONFIRMED");
        this.updateById(appointment);
        log.info("预约已确认：id={}, 医生={}", appointmentId, doctorId);
        return appointment;
    }

    @Override
    public Appointment getByIdOrNull(Long id) {
        return this.getById(id);
    }

    // ==================== 私有方法 ====================

    private void generateSlots(List<TimeSlotVO> slots, LocalTime from, LocalTime to, LocalDate date, String label) {
        LocalTime t = from;
        while (t.isBefore(to)) {
            slots.add(TimeSlotVO.builder()
                    .startTime(t)
                    .endTime(t.plusHours(1))
                    .available(true)
                    .label(label)
                    .build());
            t = t.plusHours(1);
        }
    }

    private boolean isWorkingHour(LocalTime time) {
        return (time.isAfter(MORNING_START.minusMinutes(1)) && time.isBefore(MORNING_END))
                || (time.isAfter(AFTERNOON_START.minusMinutes(1)) && time.isBefore(AFTERNOON_END));
    }

}
