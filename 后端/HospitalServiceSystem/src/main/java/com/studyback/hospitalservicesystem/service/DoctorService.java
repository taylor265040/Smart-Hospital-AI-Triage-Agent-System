package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.entity.Doctor;

import java.util.List;

/**
 * 医生服务接口
 */
public interface DoctorService extends IService<Doctor> {

    /**
     * 分页条件查询医生
     *
     * @param pageNum      页码
     * @param pageSize     每页大小
     * @param keyword      医生姓名（模糊搜索）
     * @param departmentId 所属科室ID（可选）
     * @return 分页结果
     */
    Page<Doctor> pageWithCondition(Integer pageNum, Integer pageSize, String keyword, Long departmentId);

    /**
     * 根据ID查询医生，不存在返回null
     */
    Doctor getByIdOrNull(Long id);

    /**
     * 按关键词搜索医生（供 Agent Tool 使用）
     * 匹配：医生姓名、专长、职称、简介
     *
     * @param keyword      搜索关键词
     * @param departmentId 科室ID（可选，null 时搜全部科室）
     */
    List<Doctor> search(String keyword, Long departmentId);

    /**
     * 查询某科室下所有在岗医生
     */
    List<Doctor> listByDepartment(Long departmentId);

}
