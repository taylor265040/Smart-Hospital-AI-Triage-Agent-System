package com.studyback.hospitalservicesystem.mapper;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.studyback.hospitalservicesystem.entity.UserMemory;
import org.apache.ibatis.annotations.Mapper;

/**
 * 用户长期记忆 Mapper
 */
@Mapper
public interface UserMemoryMapper extends BaseMapper<UserMemory> {
}
