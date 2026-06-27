package com.studyback.hospitalservicesystem.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.time.LocalDateTime;

/**
 * 医生实体
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@TableName("doctor")
public class Doctor implements Serializable {

    private static final long serialVersionUID = 1L;

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /** 医生姓名 */
    private String name;

    /** 职称 */
    private String title;

    /** 所属科室ID */
    private Long departmentId;

    /** 专长/擅长领域 */
    private String specialization;

    /** 医生简介 */
    private String description;

    /** 排班信息（JSON格式） */
    private String schedule;

    /** 问诊地点（如：3层呼吸内科A区3诊室） */
    private String consultationLocation;

    /** 状态：1-在岗, 0-休假/停诊 */
    private Integer status;

    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updateTime;

}
