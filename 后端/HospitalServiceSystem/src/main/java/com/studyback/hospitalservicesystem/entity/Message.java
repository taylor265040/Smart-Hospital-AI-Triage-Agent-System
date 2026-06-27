package com.studyback.hospitalservicesystem.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.time.LocalDateTime;

/**
 * 消息实体
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@TableName("message")
public class Message implements Serializable {

    private static final long serialVersionUID = 1L;

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /** 所属会话ID */
    private Long conversationId;

    /** 发送者ID */
    private Long senderId;

    /** 发送者类型：USER-用户, AI-智能客服, CS_STAFF-人工客服 */
    private String senderType;

    /** 消息内容 */
    private String content;

    /** 消息类型：TEXT-文本, IMAGE-图片 */
    private String messageType;

    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

}
