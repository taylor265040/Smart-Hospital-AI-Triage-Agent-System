package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.dto.AppointmentRequest;
import com.studyback.hospitalservicesystem.dto.TimeSlotVO;
import com.studyback.hospitalservicesystem.entity.Appointment;

import java.time.LocalDate;
import java.util.List;

/**
 * 预约挂号服务接口
 */
public interface AppointmentService extends IService<Appointment> {

    /**
     * 分页查询预约
     */
    Page<Appointment> pageWithCondition(Integer pageNum, Integer pageSize, Long userId, Long doctorId, String status);

    /**
     * 获取某医生某日的可用时间段
     * 默认工作时段：上午 8:00-12:00，下午 14:00-18:00，每小时一个slot
     */
    List<TimeSlotVO> getAvailableSlots(Long doctorId, LocalDate date);

    /**
     * 提交预约
     */
    Appointment book(AppointmentRequest request);

    /**
     * 取消预约
     */
    void cancel(Long appointmentId, Long userId);

    /**
     * 医生确认预约
     */
    Appointment confirm(Long appointmentId, Long doctorId);

    /**
     * 根据ID查询，不存在返回null
     */
    Appointment getByIdOrNull(Long id);

}
