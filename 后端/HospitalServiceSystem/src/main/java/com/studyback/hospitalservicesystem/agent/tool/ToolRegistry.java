package com.studyback.hospitalservicesystem.agent.tool;

import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import jakarta.annotation.PostConstruct;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

/**
 * 工具注册表 — 维护工具名称到执行函数的映射
 * <p>
 * Planner 输出的 toolName 通过此注册表分发到对应的 Tool 组件。
 */
@Slf4j
@Component
@RequiredArgsConstructor
public class ToolRegistry {

    private final DepartmentTool departmentTool;
    private final DoctorTool doctorTool;
    private final ScheduleTool scheduleTool;
    private final RegisterTool registerTool;
    private final ReportTool reportTool;
    private final BookingTool bookingTool;

    /** 工具名 → 执行函数（参数Map → ToolResult） */
    private final Map<String, Function<Map<String, Object>, ToolResult>> registry = new LinkedHashMap<>();

    @PostConstruct
    private void init() {
        // DEPARTMENT_TOOL
        registry.put("DEPARTMENT_TOOL", params -> {
            String action = (String) params.getOrDefault("action", "search");
            return switch (action) {
                case "getById" -> {
                    Long id = toLong(params.get("departmentId"));
                    yield departmentTool.getById(id);
                }
                case "listAll" -> departmentTool.listAll();
                default -> {
                    String keyword = (String) params.get("keyword");
                    yield departmentTool.search(keyword);
                }
            };
        });

        // DOCTOR_TOOL
        registry.put("DOCTOR_TOOL", params -> {
            String action = (String) params.getOrDefault("action", "search");
            return switch (action) {
                case "getById" -> {
                    Long id = toLong(params.get("doctorId"));
                    yield doctorTool.getById(id);
                }
                case "listByDepartment" -> {
                    Long deptId = toLong(params.get("departmentId"));
                    yield doctorTool.listByDepartment(deptId);
                }
                default -> {
                    String keyword = (String) params.get("keyword");
                    Long deptId = toLong(params.get("departmentId"));
                    yield doctorTool.search(keyword, deptId);
                }
            };
        });

        // SCHEDULE_TOOL
        registry.put("SCHEDULE_TOOL", params -> {
            Long doctorId = toLong(params.get("doctorId"));
            String doctorName = (String) params.get("doctorName");
            String date = (String) params.get("date");

            // 按姓名查找医生
            if (doctorId == null && doctorName != null && !doctorName.isBlank()) {
                if (date != null) {
                    return scheduleTool.getAvailableSlotsByName(doctorName, date);
                }
                return ToolResult.fail("SCHEDULE_TOOL", "按姓名查询排班需要提供date参数");
            }

            // 按ID查询
            if (date != null) {
                return scheduleTool.getAvailableSlots(doctorId, date);
            }
            return scheduleTool.getSchedule(doctorId);
        });

        // REGISTER_TOOL
        registry.put("REGISTER_TOOL", params -> {
            Long departmentId = toLong(params.get("departmentId"));
            Long doctorId = toLong(params.get("doctorId"));
            if (doctorId != null) {
                return registerTool.guideByDoctor(doctorId);
            }
            if (departmentId != null) {
                return registerTool.guideByDepartment(departmentId);
            }
            return registerTool.generalGuide();
        });

        // REPORT_TOOL
        registry.put("REPORT_TOOL", params -> {
            // ReportTool 主要用于内部流程，不作为独立工具调用
            return ToolResult.ok("report_tool", "导诊报告生成完成", Map.of());
        });

        // BOOKING_TOOL
        registry.put("BOOKING_TOOL", params -> {
            String action = (String) params.getOrDefault("action", "getAvailableSlots");
            return switch (action) {
                case "getAvailableSlots" -> {
                    Long doctorId = toLong(params.get("doctorId"));
                    String date = (String) params.get("date");
                    yield bookingTool.getAvailableSlots(doctorId, date);
                }
                case "listMyAppointments" -> {
                    Long userId = toLong(params.get("userId"));
                    yield bookingTool.listMyAppointments(userId);
                }
                case "book" -> {
                    Long userId = toLong(params.get("userId"));
                    Long doctorId = toLong(params.get("doctorId"));
                    Long departmentId = toLong(params.get("departmentId"));
                    String date = (String) params.get("date");
                    String startTime = (String) params.get("startTime");
                    String remark = (String) params.get("remark");
                    yield bookingTool.book(userId, doctorId, departmentId, date, startTime, remark);
                }
                case "cancel" -> {
                    Long appointmentId = toLong(params.get("appointmentId"));
                    Long userId = toLong(params.get("userId"));
                    yield bookingTool.cancel(appointmentId, userId);
                }
                default -> ToolResult.fail("BOOKING_TOOL", "未知action: " + action
                        + "，支持: getAvailableSlots, listMyAppointments, book, cancel");
            };
        });

        log.info("ToolRegistry 初始化完成，已注册 {} 个工具: {}", registry.size(), registry.keySet());
    }

    /**
     * 执行指定工具
     */
    public ToolResult execute(String toolName, Map<String, Object> params) {
        Function<Map<String, Object>, ToolResult> executor = registry.get(toolName);
        if (executor == null) {
            log.warn("无法识别的工具名称: {}，可用工具: {}", toolName, registry.keySet());
            return ToolResult.fail(toolName, "未知工具: " + toolName + "，可用工具: " + registry.keySet());
        }

        log.info("执行工具: {} params={}", toolName, params);
        try {
            ToolResult result = executor.apply(params != null ? params : Map.of());
            log.info("工具 {} 执行{}: {}", toolName, result.success() ? "成功" : "失败", result.summary());
            return result;
        } catch (Exception e) {
            log.error("工具 {} 执行异常", toolName, e);
            return ToolResult.fail(toolName, "工具执行异常: " + e.getMessage());
        }
    }

    /**
     * 获取已注册的工具名称列表
     */
    public List<String> getRegisteredToolNames() {
        return List.copyOf(registry.keySet());
    }

    /**
     * 检查工具是否存在
     */
    public boolean hasTool(String toolName) {
        return registry.containsKey(toolName);
    }

    private Long toLong(Object value) {
        if (value == null) return null;
        if (value instanceof Long l) return l;
        if (value instanceof Number n) return n.longValue();
        try {
            return Long.parseLong(value.toString());
        } catch (NumberFormatException e) {
            return null;
        }
    }
}
