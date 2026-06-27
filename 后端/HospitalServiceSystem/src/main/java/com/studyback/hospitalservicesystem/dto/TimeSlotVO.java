package com.studyback.hospitalservicesystem.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.time.LocalTime;

/**
 * 可预约时间段
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class TimeSlotVO {

    /** 开始时间 */
    private LocalTime startTime;

    /** 结束时间 */
    private LocalTime endTime;

    /** 是否可用 */
    private boolean available;

    /** 标签（如"上午"、"下午"） */
    private String label;

}
