package com.studyback.hospitalservicesystem.agent.tool;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import com.studyback.hospitalservicesystem.entity.Appointment;
import com.studyback.hospitalservicesystem.entity.Doctor;
import com.studyback.hospitalservicesystem.service.AppointmentService;
import com.studyback.hospitalservicesystem.service.DoctorService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.time.DayOfWeek;
import java.time.LocalDate;
import java.time.LocalTime;
import java.time.format.DateTimeFormatter;
import java.util.*;

/**
 * 排班查询工具
 * <p>
 * 查询医生排班信息和可用号源。
 */
@Slf4j
@Component("agentScheduleTool")
@RequiredArgsConstructor
public class ScheduleTool {

    private final DoctorService doctorService;
    private final AppointmentService appointmentService;
    private final ObjectMapper objectMapper;

    /**
     * 查询医生的排班信息
     */
    public ToolResult getSchedule(Long doctorId) {
        Doctor doc = doctorService.getByIdOrNull(doctorId);
        if (doc == null) {
            return ToolResult.fail("schedule_get", "医生不存在（ID=" + doctorId + "）");
        }

        Map<String, Object> data = new LinkedHashMap<>();
        data.put("doctorId", doc.getId());
        data.put("doctorName", doc.getName());
        data.put("title", doc.getTitle());

        // 尝试解析 schedule JSON
        String scheduleJson = doc.getSchedule();
        if (scheduleJson != null && !scheduleJson.isBlank()) {
            try {
                JsonNode scheduleNode = objectMapper.readTree(scheduleJson);
                data.put("schedule", objectMapper.treeToValue(scheduleNode, Map.class));
                return ToolResult.ok("schedule_get",
                        doc.getName() + " " + doc.getTitle() + " — 有排班信息",
                        data);
            } catch (Exception e) {
                log.warn("解析医生 {} 排班 JSON 失败", doc.getId());
            }
        }

        // 无排班 JSON，返回基本信息
        data.put("schedule", "暂无详细排班信息，建议咨询医院前台");
        return ToolResult.ok("schedule_get",
                doc.getName() + " " + doc.getTitle() + " — 排班信息待确认",
                data);
    }

    /**
     * 查询医生某日的可用时段（按医生姓名查找）
     */
    public ToolResult getAvailableSlotsByName(String doctorName, String dateStr) {
        if (doctorName == null || doctorName.isBlank()) {
            return ToolResult.fail("schedule_slots", "请提供医生姓名");
        }
        // 按姓名搜索医生
        List<Doctor> doctors = doctorService.search(doctorName, null);
        if (doctors.isEmpty()) {
            return ToolResult.fail("schedule_slots", "未找到医生：" + doctorName);
        }
        // 取第一个匹配的医生
        Doctor doc = doctors.get(0);
        return getAvailableSlots(doc.getId(), dateStr);
    }

    /**
     * 查询医生某日的可用时段
     */
    public ToolResult getAvailableSlots(Long doctorId, String dateStr) {
        Doctor doc = doctorService.getByIdOrNull(doctorId);
        if (doc == null) {
            return ToolResult.fail("schedule_slots", "医生不存在（ID=" + doctorId + "）");
        }

        try {
            LocalDate date = LocalDate.parse(dateStr, DateTimeFormatter.ISO_LOCAL_DATE);
            LocalDate today = LocalDate.now();

            if (date.isBefore(today)) {
                return ToolResult.fail("schedule_slots", "无法查询过去的日期");
            }

            // 检查医生状态
            if (doc.getStatus() != null && doc.getStatus() == 0) {
                return ToolResult.ok("schedule_slots",
                        doc.getName() + "目前休假中，暂无号源",
                        Map.of("doctorId", doctorId, "doctorName", doc.getName(),
                                "date", dateStr, "available", false, "reason", "医生休假"));
            }

            // 尝试查询已有预约
            List<Appointment> existingApps = appointmentService.list(
                    new com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper<Appointment>()
                            .eq(Appointment::getDoctorId, doctorId)
                            .eq(Appointment::getAppointmentDate, date)
                            .in(Appointment::getStatus, "PENDING", "CONFIRMED")
            );

            // 工作时间段
            List<Map<String, Object>> slots = buildTimeSlots(date);
            Set<String> bookedStartTimes = new HashSet<>();
            for (Appointment app : existingApps) {
                if (app.getStartTime() != null) {
                    bookedStartTimes.add(app.getStartTime().toString());
                }
            }

            int availableCount = 0;
            for (Map<String, Object> slot : slots) {
                boolean isBooked = bookedStartTimes.contains(slot.get("startTime"));
                slot.put("available", !isBooked);
                if (!isBooked) availableCount++;
            }

            String summary = dateStr + " " + doc.getName() + " " + doc.getTitle()
                    + "：共 " + slots.size() + " 个时段，" + availableCount + " 个可用";
            Map<String, Object> data = new LinkedHashMap<>();
            data.put("doctorId", doctorId);
            data.put("doctorName", doc.getName());
            data.put("title", doc.getTitle());
            data.put("date", dateStr);
            data.put("totalSlots", slots.size());
            data.put("availableSlots", availableCount);
            data.put("slots", slots);

            return ToolResult.ok("schedule_slots", summary, data);

        } catch (Exception e) {
            log.error("查询排班失败", e);
            return ToolResult.fail("schedule_slots", "查询排班失败：" + e.getMessage());
        }
    }

    private List<Map<String, Object>> buildTimeSlots(LocalDate date) {
        List<Map<String, Object>> slots = new ArrayList<>();
        int[][] hours = {{8, 12}, {14, 18}};
        String[] labels = {"上午", "下午"};

        LocalDate today = LocalDate.now();
        LocalTime now = LocalTime.now();

        for (int g = 0; g < hours.length; g++) {
            for (int h = hours[g][0]; h < hours[g][1]; h++) {
                LocalTime start = LocalTime.of(h, 0);
                LocalTime end = LocalTime.of(h + 1, 0);
                // 过滤已过去的时间段
                if (date.isEqual(today) && end.isBefore(now.plusMinutes(30))) {
                    continue;
                }
                Map<String, Object> slot = new LinkedHashMap<>();
                slot.put("startTime", start.toString());
                slot.put("endTime", end.toString());
                slot.put("label", labels[g]);
                slots.add(slot);
            }
        }
        return slots;
    }
}
