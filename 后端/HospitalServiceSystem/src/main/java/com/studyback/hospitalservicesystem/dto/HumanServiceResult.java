package com.studyback.hospitalservicesystem.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 人工客服操作结果
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class HumanServiceResult {

    /** 是否成功 */
    private boolean success;

    /** 提示信息 */
    private String message;

    /** 当前排队位置（0=已被接听） */
    private int queuePosition;

    /** 前方等待人数 */
    private int waitingCount;

    /** 分配的客服人员姓名（已接听时有值） */
    private String csStaffName;

    /** 会话ID */
    private Long conversationId;

}
