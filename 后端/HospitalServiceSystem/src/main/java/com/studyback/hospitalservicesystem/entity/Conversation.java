package com.studyback.hospitalservicesystem.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.time.LocalDateTime;

/**
 * 会话实体
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@TableName("conversation")
public class Conversation implements Serializable {

    private static final long serialVersionUID = 1L;

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /** 用户ID */
    private Long userId;

    /** 客服人员ID（人工客服接管时填写） */
    private Long csStaffId;

    /** 会话类型：AI-智能客服, MANUAL-人工客服 */
    private String type;

    /** 会话状态：ACTIVE-活跃, CLOSED-已关闭 */
    private String status;

    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updateTime;

}
