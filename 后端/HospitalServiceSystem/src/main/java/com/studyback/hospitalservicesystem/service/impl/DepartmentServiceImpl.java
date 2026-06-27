package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.studyback.hospitalservicesystem.entity.Department;
import com.studyback.hospitalservicesystem.mapper.DepartmentMapper;
import com.studyback.hospitalservicesystem.service.DepartmentService;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import java.util.List;

/**
 * 科室服务实现
 */
@Service
public class DepartmentServiceImpl extends ServiceImpl<DepartmentMapper, Department> implements DepartmentService {

    @Override
    public Page<Department> pageWithCondition(Integer pageNum, Integer pageSize, String keyword) {
        Page<Department> page = new Page<>(pageNum, pageSize);
        LambdaQueryWrapper<Department> wrapper = new LambdaQueryWrapper<>();

        if (StringUtils.hasText(keyword)) {
            wrapper.like(Department::getName, keyword);
        }
        wrapper.orderByAsc(Department::getSortOrder)
                .orderByDesc(Department::getCreateTime);

        return this.page(page, wrapper);
    }

    @Override
    public List<Department> listEnabled() {
        LambdaQueryWrapper<Department> wrapper = new LambdaQueryWrapper<>();
        wrapper.eq(Department::getStatus, 1)
                .orderByAsc(Department::getSortOrder);
        return this.list(wrapper);
    }

    @Override
    public Department getByIdOrNull(Long id) {
        return this.getById(id);
    }

    @Override
    public boolean existsByName(String name, Long excludeId) {
        LambdaQueryWrapper<Department> wrapper = new LambdaQueryWrapper<>();
        wrapper.eq(Department::getName, name);
        if (excludeId != null) {
            wrapper.ne(Department::getId, excludeId);
        }
        return this.count(wrapper) > 0;
    }

    @Override
    public List<Department> search(String keyword) {
        if (!StringUtils.hasText(keyword)) {
            return listEnabled();
        }
        LambdaQueryWrapper<Department> wrapper = new LambdaQueryWrapper<>();
        wrapper.and(w -> w
                        .like(Department::getName, keyword)
                        .or().like(Department::getDescription, keyword)
                        .or().like(Department::getFloor, keyword)
                        .or().like(Department::getLocation, keyword))
                .eq(Department::getStatus, 1)
                .orderByAsc(Department::getSortOrder);
        return this.list(wrapper);
    }

}
