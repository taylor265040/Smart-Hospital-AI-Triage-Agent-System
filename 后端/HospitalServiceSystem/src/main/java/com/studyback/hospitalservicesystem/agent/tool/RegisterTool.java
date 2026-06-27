package com.studyback.hospitalservicesystem.agent.tool;

import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import com.studyback.hospitalservicesystem.entity.Department;
import com.studyback.hospitalservicesystem.entity.Doctor;
import com.studyback.hospitalservicesystem.service.DepartmentService;
import com.studyback.hospitalservicesystem.service.DoctorService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * 挂号指引工具
 * <p>
 * 提供预约挂号流程指导和入口信息，不直接完成预约（仅做指引）。
 */
@Slf4j
@Component("agentRegisterTool")
@RequiredArgsConstructor
public class RegisterTool {

    private final DepartmentService departmentService;
    private final DoctorService doctorService;

    private static final String REGISTRATION_GUIDE =
            "预约挂号方式：\n"
                    + "1. 微信公众号：关注[XX医院]公众号 -> 就医服务 -> 预约挂号\n"
                    + "2. 医院APP：下载[XX医院]APP -> 预约挂号\n"
                    + "3. 官网：访问 www.xxx-hospital.com -> 在线预约\n"
                    + "4. 电话预约：拨打 010-12345678\n"
                    + "5. 现场挂号：门诊楼1层挂号窗口\n\n"
                    + "提示：建议提前1-3天预约，当天号源有限。";

    /**
     * 获取科室挂号指引
     */
    public ToolResult guideByDepartment(Long departmentId) {
        Department dept = departmentService.getByIdOrNull(departmentId);
        if (dept == null) {
            return ToolResult.fail("register_guide", "科室不存在（ID=" + departmentId + "）");
        }

        List<Doctor> doctors = doctorService.listByDepartment(departmentId);

        Map<String, Object> data = new LinkedHashMap<>();
        data.put("departmentId", dept.getId());
        data.put("departmentName", dept.getName());
        data.put("departmentLocation", dept.getLocation());
        data.put("doctorsCount", doctors.size());
        data.put("registrationGuide", REGISTRATION_GUIDE);

        if (!doctors.isEmpty()) {
            List<Map<String, String>> docBrief = doctors.stream()
                    .map(d -> Map.of("name", d.getName(), "title",
                            d.getTitle() != null ? d.getTitle() : ""))
                    .toList();
            data.put("availableDoctors", docBrief);
        }

        String summary = "已为您查询" + dept.getName() + "挂号信息。"
                + "该科室位于" + dept.getLocation() + "，"
                + "共有 " + doctors.size() + " 位医生在岗。";

        return ToolResult.ok("register_guide", summary, data);
    }

    /**
     * 获取通用挂号指引
     */
    public ToolResult generalGuide() {
        Map<String, Object> data = new LinkedHashMap<>();
        data.put("registrationGuide", REGISTRATION_GUIDE);
        data.put("appointmentUrl", "/api/appointment");

        return ToolResult.ok("register_guide",
                "已为您提供挂号流程指引，请选择合适的渠道预约。",
                data);
    }

    /**
     * 根据医生获取挂号指引
     */
    public ToolResult guideByDoctor(Long doctorId) {
        Doctor doc = doctorService.getByIdOrNull(doctorId);
        if (doc == null) {
            return ToolResult.fail("register_guide", "医生不存在（ID=" + doctorId + "）");
        }

        Long deptId = doc.getDepartmentId();
        Map<String, Object> data = new LinkedHashMap<>();
        data.put("doctorId", doc.getId());
        data.put("doctorName", doc.getName());
        data.put("title", doc.getTitle());
        data.put("registrationGuide", REGISTRATION_GUIDE);

        if (deptId != null) {
            Department dept = departmentService.getByIdOrNull(deptId);
            if (dept != null) {
                data.put("departmentName", dept.getName());
                data.put("departmentLocation", dept.getLocation());
            }
        }

        return ToolResult.ok("register_guide",
                "已为您查询" + doc.getName() + " " + doc.getTitle() + "的挂号信息。"
                        + "请选择合适的渠道预约。",
                data);
    }
}
