package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.studyback.hospitalservicesystem.entity.Doctor;
import com.studyback.hospitalservicesystem.mapper.DoctorMapper;
import com.studyback.hospitalservicesystem.service.DoctorService;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import java.util.List;

/**
 * 医生服务实现
 */
@Service
public class DoctorServiceImpl extends ServiceImpl<DoctorMapper, Doctor> implements DoctorService {

    @Override
    public Page<Doctor> pageWithCondition(Integer pageNum, Integer pageSize, String keyword, Long departmentId) {
        Page<Doctor> page = new Page<>(pageNum, pageSize);
        LambdaQueryWrapper<Doctor> wrapper = new LambdaQueryWrapper<>();

        if (StringUtils.hasText(keyword)) {
            wrapper.like(Doctor::getName, keyword);
        }
        if (departmentId != null) {
            wrapper.eq(Doctor::getDepartmentId, departmentId);
        }
        wrapper.orderByDesc(Doctor::getCreateTime);

        return this.page(page, wrapper);
    }

    @Override
    public Doctor getByIdOrNull(Long id) {
        return this.getById(id);
    }

    @Override
    public List<Doctor> search(String keyword, Long departmentId) {
        LambdaQueryWrapper<Doctor> wrapper = new LambdaQueryWrapper<>();
        wrapper.eq(Doctor::getStatus, 1);
        if (StringUtils.hasText(keyword)) {
            wrapper.and(w -> w
                    .like(Doctor::getName, keyword)
                    .or().like(Doctor::getSpecialization, keyword)
                    .or().like(Doctor::getTitle, keyword)
                    .or().like(Doctor::getDescription, keyword));
        }
        if (departmentId != null) {
            wrapper.eq(Doctor::getDepartmentId, departmentId);
        }
        wrapper.orderByDesc(Doctor::getCreateTime);
        return this.list(wrapper);
    }

    @Override
    public List<Doctor> listByDepartment(Long departmentId) {
        return search(null, departmentId);
    }

}
