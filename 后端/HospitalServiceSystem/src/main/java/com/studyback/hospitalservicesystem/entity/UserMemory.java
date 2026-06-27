package com.studyback.hospitalservicesystem.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.time.LocalDateTime;

/**
 * 用户长期记忆实体
 * <p>
 * Agent 通过此表持久化用户偏好、病史、上次就诊信息等。
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@TableName("user_memory")
public class UserMemory implements Serializable {

    private static final long serialVersionUID = 1L;

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /** 用户ID */
    private Long userId;

    /** 记忆键（如 preferred_doctor, medical_history, last_department） */
    private String memoryKey;

    /** 记忆值 */
    private String memoryValue;

    /** 过期时间（NULL 表示永不过期） */
    private LocalDateTime expireTime;

    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updateTime;
}
