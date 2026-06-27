package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.entity.User;

/**
 * 用户服务接口
 */
public interface UserService extends IService<User> {

    /**
     * 分页条件查询用户
     */
    Page<User> pageWithCondition(Integer pageNum, Integer pageSize, String keyword, String role);

    /**
     * 根据ID查询用户，不存在返回null
     */
    User getByIdOrNull(Long id);

    /**
     * 校验用户名是否已存在
     */
    boolean existsByUsername(String username, Long excludeId);

}
