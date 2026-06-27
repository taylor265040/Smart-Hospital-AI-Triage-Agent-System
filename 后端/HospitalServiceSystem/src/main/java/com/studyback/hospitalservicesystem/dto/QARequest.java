package com.studyback.hospitalservicesystem.dto;

import jakarta.validation.constraints.NotBlank;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 智能问答请求
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class QARequest {

    /** 用户问题 */
    @NotBlank(message = "问题不能为空")
    private String question;

    /** 用户ID */
    private Long userId;

    /** 会话ID（可选，关联历史对话） */
    private Long conversationId;

}
