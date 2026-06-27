package com.studyback.hospitalservicesystem.dto;

import jakarta.validation.constraints.NotNull;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.time.LocalDate;
import java.time.LocalTime;

/**
 * 预约挂号请求
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class AppointmentRequest {

    @NotNull(message = "用户ID不能为空")
    private Long userId;

    @NotNull(message = "医生ID不能为空")
    private Long doctorId;

    @NotNull(message = "科室ID不能为空")
    private Long departmentId;

    @NotNull(message = "预约日期不能为空")
    private LocalDate appointmentDate;

    @NotNull(message = "预约时间不能为空")
    private LocalTime startTime;

    /** 备注 */
    private String remark;

}
