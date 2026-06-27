package com.studyback.hospitalservicesystem.mapper;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.studyback.hospitalservicesystem.entity.Appointment;
import org.apache.ibatis.annotations.Mapper;

/**
 * 预约挂号 Mapper
 */
@Mapper
public interface AppointmentMapper extends BaseMapper<Appointment> {

}
