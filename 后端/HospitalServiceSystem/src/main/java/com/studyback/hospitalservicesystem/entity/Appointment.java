package com.studyback.hospitalservicesystem.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.time.LocalDate;
import java.time.LocalTime;
import java.time.LocalDateTime;

/**
 * 预约挂号实体
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@TableName("appointment")
public class Appointment implements Serializable {

    private static final long serialVersionUID = 1L;

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /** 用户ID */
    private Long userId;

    /** 医生ID */
    private Long doctorId;

    /** 科室ID */
    private Long departmentId;

    /** 预约日期 */
    private LocalDate appointmentDate;

    /** 开始时间（整点，如 09:00） */
    private LocalTime startTime;

    /** 结束时间（开始+1小时） */
    private LocalTime endTime;

    /** 状态：PENDING-待确认, CONFIRMED-已确认, CANCELLED-已取消 */
    private String status;

    /** 备注 */
    private String remark;

    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updateTime;

}
