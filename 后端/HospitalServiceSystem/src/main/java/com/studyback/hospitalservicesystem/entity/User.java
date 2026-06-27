package com.studyback.hospitalservicesystem.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.time.LocalDateTime;

/**
 * 用户实体
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@TableName("user")
public class User implements Serializable {

    private static final long serialVersionUID = 1L;

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /** 用户名 */
    private String username;

    /** 密码 */
    private String password;

    /** 真实姓名 */
    private String realName;

    /** 手机号 */
    private String phone;

    /** 邮箱 */
    private String email;

    /** 角色：PATIENT-患者, CS_STAFF-客服人员, ADMIN-系统管理员, DOCTOR-医生 */
    private String role;

    /** 头像URL */
    private String avatar;

    /** 状态：1-启用, 0-禁用 */
    private Integer status;

    /** 客服在线状态：1-在线, 0-离线（仅 CS_STAFF 角色使用） */
    private Integer isOnline;

    /** 逻辑删除标识：1-已删除, 0-未删除 */
    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updateTime;

}
