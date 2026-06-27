package com.studyback.hospitalservicesystem.mapper;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.studyback.hospitalservicesystem.entity.Doctor;
import org.apache.ibatis.annotations.Mapper;

/**
 * 医生 Mapper 接口
 */
@Mapper
public interface DoctorMapper extends BaseMapper<Doctor> {

}
