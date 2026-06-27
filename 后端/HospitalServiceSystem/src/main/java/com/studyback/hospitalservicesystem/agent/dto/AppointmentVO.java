package com.studyback.hospitalservicesystem.agent.dto;

import lombok.Builder;

/**
 * 预约信息视图对象 — 供 BookingTool 返回给 LLM 的结构化数据
 */
@Builder
public record AppointmentVO(
        Long id,
        String doctorName,
        String departmentName,
        String appointmentDate,
        String startTime,
        String endTime,
        String status
) {
    public String toSummary() {
        return doctorName + " " + departmentName + " " + appointmentDate + " " + startTime + "-" + endTime
                + "（" + ("PENDING".equals(status) ? "待确认" : "CONFIRMED".equals(status) ? "已确认" : "已取消") + "）";
    }
}
