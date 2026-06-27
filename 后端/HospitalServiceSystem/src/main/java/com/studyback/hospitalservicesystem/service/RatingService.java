package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.entity.Rating;

/**
 * 满意度评价服务接口
 */
public interface RatingService extends IService<Rating> {

    /**
     * 分页条件查询评价
     */
    Page<Rating> pageWithCondition(Integer pageNum, Integer pageSize, Long userId, Integer minScore);

    /**
     * 根据ID查询评价，不存在返回null
     */
    Rating getByIdOrNull(Long id);

}
