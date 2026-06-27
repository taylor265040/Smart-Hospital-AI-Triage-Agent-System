package com.studyback.hospitalservicesystem.agent.tool;

import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import com.studyback.hospitalservicesystem.entity.Doctor;
import com.studyback.hospitalservicesystem.service.DoctorService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 医生查询工具
 * <p>
 * 按关键词搜索医生，或查询某科室下在岗医生列表。
 */
@Slf4j
@Component("agentDoctorTool")
@RequiredArgsConstructor
public class DoctorTool {

    private final DoctorService doctorService;

    /**
     * 搜索医生
     *
     * @param keyword      搜索关键词（姓名/专长/职称）
     * @param departmentId 科室ID（可选）
     */
    public ToolResult search(String keyword, Long departmentId) {
        List<Doctor> results = doctorService.search(keyword, departmentId);

        if (results.isEmpty()) {
            String hint = departmentId != null ? "该科室暂无在岗医生" : "未找到匹配的医生";
            return ToolResult.ok("doctor_search", hint, Map.of("count", 0, "doctors", List.of()));
        }

        List<Map<String, Object>> docList = results.stream()
                .map(this::toMap)
                .collect(Collectors.toList());

        String summary = results.size() == 1
                ? results.get(0).getName() + " " + results.get(0).getTitle()
                + "，擅长：" + results.get(0).getSpecialization()
                : "找到 " + results.size() + " 位医生";

        return ToolResult.ok("doctor_search", summary,
                Map.of("count", results.size(), "doctors", docList));
    }

    /**
     * 根据ID查询医生详情
     */
    public ToolResult getById(Long id) {
        Doctor doc = doctorService.getByIdOrNull(id);
        if (doc == null) {
            return ToolResult.fail("doctor_get", "医生不存在（ID=" + id + "）");
        }

        Map<String, Object> data = toMap(doc);
        return ToolResult.ok("doctor_get",
                doc.getName() + " " + doc.getTitle(), data);
    }

    /**
     * 查询某科室所有在岗医生
     */
    public ToolResult listByDepartment(Long departmentId) {
        return search(null, departmentId);
    }

    private Map<String, Object> toMap(Doctor d) {
        Map<String, Object> m = new LinkedHashMap<>();
        m.put("id", d.getId());
        m.put("name", d.getName());
        m.put("title", d.getTitle());
        m.put("departmentId", d.getDepartmentId());
        m.put("specialization", d.getSpecialization());
        m.put("description", d.getDescription());
        m.put("schedule", d.getSchedule());
        m.put("status", d.getStatus());
        return m;
    }
}
