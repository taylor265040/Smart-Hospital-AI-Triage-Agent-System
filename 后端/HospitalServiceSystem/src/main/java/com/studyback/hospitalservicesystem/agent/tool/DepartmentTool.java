package com.studyback.hospitalservicesystem.agent.tool;

import com.studyback.hospitalservicesystem.agent.dto.ToolResult;
import com.studyback.hospitalservicesystem.entity.Department;
import com.studyback.hospitalservicesystem.service.DepartmentService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 科室查询工具
 * <p>
 * 根据关键词搜索科室，或返回全部启用科室列表。
 */
@Slf4j
@Component("agentDepartmentTool")
@RequiredArgsConstructor
public class DepartmentTool {

    private final DepartmentService departmentService;

    /**
     * 搜索科室
     *
     * @param keyword 搜索关键词（可选，为空时返回全部启用科室）
     */
    public ToolResult search(String keyword) {
        List<Department> results = departmentService.search(keyword);

        if (results.isEmpty()) {
            return ToolResult.ok("department_search",
                    "未找到匹配的科室",
                    Map.of("count", 0, "departments", List.of()));
        }

        List<Map<String, Object>> deptList = results.stream()
                .map(d -> {
                    Map<String, Object> m = new LinkedHashMap<>();
                    m.put("id", d.getId());
                    m.put("name", d.getName());
                    m.put("floor", d.getFloor());
                    m.put("location", d.getLocation());
                    m.put("description", d.getDescription());
                    m.put("phone", d.getPhone());
                    return m;
                })
                .collect(Collectors.toList());

        String summary = results.size() == 1
                ? "科室：" + results.get(0).getName() + "（" + results.get(0).getLocation() + "）"
                : "找到 " + results.size() + " 个科室";

        return ToolResult.ok("department_search", summary,
                Map.of("count", results.size(), "departments", deptList));
    }

    /**
     * 根据ID查询科室详情
     */
    public ToolResult getById(Long id) {
        Department dept = departmentService.getByIdOrNull(id);
        if (dept == null) {
            return ToolResult.fail("department_get", "科室不存在（ID=" + id + "）");
        }

        Map<String, Object> data = new LinkedHashMap<>();
        data.put("id", dept.getId());
        data.put("name", dept.getName());
        data.put("floor", dept.getFloor());
        data.put("location", dept.getLocation());
        data.put("description", dept.getDescription());
        data.put("phone", dept.getPhone());

        return ToolResult.ok("department_get",
                dept.getName() + "（" + dept.getLocation() + "）", data);
    }

    /**
     * 获取全部启用科室
     */
    public ToolResult listAll() {
        return search(null);
    }
}
