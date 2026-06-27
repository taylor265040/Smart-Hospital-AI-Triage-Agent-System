package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.entity.Department;

import java.util.List;

/**
 * 科室服务接口
 */
public interface DepartmentService extends IService<Department> {

    /**
     * 分页条件查询科室
     *
     * @param pageNum  页码
     * @param pageSize 每页大小
     * @param keyword  科室名称（模糊搜索，可选）
     * @return 分页结果
     */
    Page<Department> pageWithCondition(Integer pageNum, Integer pageSize, String keyword);

    /**
     * 查询所有启用的科室
     *
     * @return 启用科室列表（按排序字段升序）
     */
    List<Department> listEnabled();

    /**
     * 根据ID查询科室（找不到返回 null，由调用方处理）
     *
     * @param id 科室ID
     * @return 科室实体或 null
     */
    Department getByIdOrNull(Long id);

    /**
     * 重名校验：同一名称的科室是否已存在
     *
     * @param name       科室名称
     * @param excludeId  排除的ID（更新时排除自身）
     * @return true-已存在, false-不存在
     */
    boolean existsByName(String name, Long excludeId);

    /**
     * 按关键词搜索科室（供 Agent Tool 使用）
     * 匹配：科室名称、描述、楼层、位置
     */
    List<Department> search(String keyword);

}
