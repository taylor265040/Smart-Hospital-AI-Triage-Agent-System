package com.studyback.hospitalservicesystem.agent.tool;

import com.studyback.hospitalservicesystem.agent.dto.AppointmentVO;
import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import com.studyback.hospitalservicesystem.dto.AppointmentRequest;
import com.studyback.hospitalservicesystem.dto.TimeSlotVO;
import com.studyback.hospitalservicesystem.entity.Appointment;
import com.studyback.hospitalservicesystem.entity.Department;
import com.studyback.hospitalservicesystem.entity.Doctor;
import com.studyback.hospitalservicesystem.service.AppointmentService;
import com.studyback.hospitalservicesystem.service.DepartmentService;
import com.studyback.hospitalservicesystem.service.DoctorService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.time.LocalDate;
import java.time.LocalTime;
import java.time.format.DateTimeFormatter;
import java.util.*;

/**
 * 智能挂号工具 — 真正执行预约操作
 * <p>
 * 区别于 RegisterTool（仅返回静态指引），此工具调用 AppointmentService
 * 完成号源查询、预约提交、取消等实际操作。
 */
@Slf4j
@Component("agentBookingTool")
@RequiredArgsConstructor
public class BookingTool {

    private final AppointmentService appointmentService;
    private final DoctorService doctorService;
    private final DepartmentService departmentService;

    private static final DateTimeFormatter DATE_FMT = DateTimeFormatter.ofPattern("yyyy-MM-dd");

    /**
     * 查询某医生某日可用时段
     */
    public ToolResult getAvailableSlots(Long doctorId, String dateStr) {
        Doctor doc = doctorService.getByIdOrNull(doctorId);
        if (doc == null) {
            return ToolResult.fail("booking_slots", "医生不存在（ID=" + doctorId + "）");
        }

        try {
            LocalDate date = LocalDate.parse(dateStr, DATE_FMT);
            if (date.isBefore(LocalDate.now())) {
                return ToolResult.fail("booking_slots", "无法预约过去的日期");
            }

            // 检查医生是否休假
            if (doc.getStatus() != null && doc.getStatus() == 0) {
                Map<String, Object> data = new LinkedHashMap<>();
                data.put("doctorId", doctorId);
                data.put("doctorName", doc.getName());
                data.put("available", false);
                data.put("reason", "医生休假中");
                return ToolResult.ok("booking_slots",
                        doc.getName() + " " + doc.getTitle() + " 目前休假中，暂无号源", data);
            }

            List<TimeSlotVO> allSlots = appointmentService.getAvailableSlots(doctorId, date);
            long availableCount = allSlots.stream().filter(TimeSlotVO::isAvailable).count();

            // 格式化时段列表（分上午/下午）
            StringBuilder summary = new StringBuilder();
            summary.append(doc.getName()).append(" ").append(doc.getTitle())
                    .append(" — ").append(dateStr).append("\n");

            String currentLabel = "";
            for (TimeSlotVO s : allSlots) {
                if (!s.getLabel().equals(currentLabel)) {
                    currentLabel = s.getLabel();
                    summary.append("【").append(currentLabel).append("】");
                }
                if (s.isAvailable()) {
                    summary.append(s.getStartTime().toString()).append(" ");
                }
            }

            Map<String, Object> data = new LinkedHashMap<>();
            data.put("doctorId", doctorId);
            data.put("doctorName", doc.getName());
            data.put("title", doc.getTitle());
            data.put("date", dateStr);
            data.put("totalSlots", allSlots.size());
            data.put("availableSlots", (int) availableCount);

            List<Map<String, Object>> slotList = new ArrayList<>();
            for (TimeSlotVO s : allSlots) {
                Map<String, Object> slot = new LinkedHashMap<>();
                slot.put("startTime", s.getStartTime().toString());
                slot.put("endTime", s.getEndTime().toString());
                slot.put("available", s.isAvailable());
                slot.put("label", s.getLabel());
                slotList.add(slot);
            }
            data.put("slots", slotList);

            String summaryStr = availableCount > 0
                    ? doc.getName() + " " + doc.getTitle() + " " + dateStr
                        + " 共 " + availableCount + " 个可用时段"
                    : "很抱歉，" + doc.getName() + " " + dateStr + " 没有可用时段";

            return ToolResult.ok("booking_slots", summaryStr, data);

        } catch (Exception e) {
            log.error("查询可用时段失败", e);
            return ToolResult.fail("booking_slots", "查询失败：" + e.getMessage());
        }
    }

    /**
     * 查询用户的有效预约
     */
    public ToolResult listMyAppointments(Long userId) {
        try {
            var page = appointmentService.pageWithCondition(1, 50, userId, null, null);
            List<Appointment> apps = page.getRecords();

            if (apps.isEmpty()) {
                return ToolResult.ok("booking_list", "您目前没有预约记录",
                        Map.of("count", 0, "appointments", List.of()));
            }

            List<Map<String, Object>> appList = new ArrayList<>();
            StringBuilder summary = new StringBuilder("您目前有 " + apps.size() + " 笔预约：\n");

            for (int i = 0; i < apps.size(); i++) {
                Appointment a = apps.get(i);
                Doctor doc = doctorService.getByIdOrNull(a.getDoctorId());
                Department dept = departmentService.getByIdOrNull(a.getDepartmentId());

                String docName = doc != null ? doc.getName() : "医生#" + a.getDoctorId();
                String deptName = dept != null ? dept.getName() : "科室#" + a.getDepartmentId();
                String statusLabel = switch (a.getStatus()) {
                    case "PENDING" -> "待确认";
                    case "CONFIRMED" -> "已确认";
                    case "CANCELLED" -> "已取消";
                    default -> a.getStatus();
                };

                summary.append("  ").append(i + 1).append(". ")
                        .append(docName).append(" ").append(deptName).append(" ")
                        .append(a.getAppointmentDate()).append(" ")
                        .append(a.getStartTime()).append("-").append(a.getEndTime())
                        .append(" [").append(statusLabel).append("]\n");

                Map<String, Object> m = new LinkedHashMap<>();
                m.put("id", a.getId());
                m.put("doctorName", docName);
                m.put("departmentName", deptName);
                m.put("appointmentDate", a.getAppointmentDate().toString());
                m.put("startTime", a.getStartTime().toString());
                m.put("endTime", a.getEndTime() != null ? a.getEndTime().toString() : "");
                m.put("status", a.getStatus());
                m.put("statusLabel", statusLabel);
                appList.add(m);
            }

            Map<String, Object> data = new LinkedHashMap<>();
            data.put("count", apps.size());
            data.put("appointments", appList);

            return ToolResult.ok("booking_list", summary.toString().trim(), data);

        } catch (Exception e) {
            log.error("查询预约列表失败", e);
            return ToolResult.fail("booking_list", "查询失败：" + e.getMessage());
        }
    }

    /**
     * 执行预约
     */
    public ToolResult book(Long userId, Long doctorId, Long departmentId,
                           String dateStr, String startTimeStr, String remark) {
        try {
            LocalDate date = LocalDate.parse(dateStr, DATE_FMT);
            LocalTime startTime = LocalTime.parse(startTimeStr);

            AppointmentRequest request = new AppointmentRequest();
            request.setUserId(userId);
            request.setDoctorId(doctorId);
            request.setDepartmentId(departmentId);
            request.setAppointmentDate(date);
            request.setStartTime(startTime);
            request.setRemark(remark);

            Appointment appointment = appointmentService.book(request);

            // 构建成功响应
            Doctor doc = doctorService.getByIdOrNull(doctorId);
            Department dept = departmentService.getByIdOrNull(departmentId);
            String docName = doc != null ? doc.getName() : "医生#" + doctorId;
            String docTitle = doc != null ? doc.getTitle() : "";
            String deptName = dept != null ? dept.getName() : "科室#" + departmentId;
            String location = dept != null ? dept.getLocation() : "";

            String summary = "预约成功！" + docName + " " + docTitle + "，" + deptName + "，"
                    + dateStr + " " + startTimeStr + "-"
                    + startTime.plusHours(1) + "，"
                    + (location.isBlank() ? "" : "就诊地点：" + location + "。")
                    + "请按时就诊。";

            Map<String, Object> data = new LinkedHashMap<>();
            data.put("action", "book");
            data.put("appointmentId", appointment.getId());
            data.put("doctorName", docName);
            data.put("doctorTitle", docTitle);
            data.put("departmentName", deptName);
            data.put("departmentLocation", location);
            data.put("date", dateStr);
            data.put("startTime", startTimeStr);
            data.put("endTime", startTime.plusHours(1).toString());
            data.put("status", "PENDING");
            data.put("appointmentSummary", summary);

            log.info("BookingTool 预约成功: userId={}, doctor={}, date={}, time={}",
                    userId, docName, dateStr, startTimeStr);

            return ToolResult.ok("booking_book", summary, data);

        } catch (IllegalArgumentException e) {
            log.warn("BookingTool 预约失败（业务规则）: {}", e.getMessage());
            return ToolResult.fail("booking_book", e.getMessage());
        } catch (Exception e) {
            log.error("BookingTool 预约异常", e);
            return ToolResult.fail("booking_book", "预约失败：" + e.getMessage());
        }
    }

    /**
     * 取消预约
     */
    public ToolResult cancel(Long appointmentId, Long userId) {
        try {
            appointmentService.cancel(appointmentId, userId);

            Map<String, Object> data = new LinkedHashMap<>();
            data.put("action", "cancel");
            data.put("appointmentId", appointmentId);

            log.info("BookingTool 取消预约成功: appointmentId={}, userId={}", appointmentId, userId);

            return ToolResult.ok("booking_cancel", "已成功取消预约。如有需要可以重新预约。", data);

        } catch (IllegalArgumentException e) {
            log.warn("BookingTool 取消失败（业务规则）: {}", e.getMessage());
            return ToolResult.fail("booking_cancel", e.getMessage());
        } catch (Exception e) {
            log.error("BookingTool 取消预约异常", e);
            return ToolResult.fail("booking_cancel", "取消失败：" + e.getMessage());
        }
    }
}
