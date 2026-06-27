package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.studyback.hospitalservicesystem.entity.Rating;
import com.studyback.hospitalservicesystem.mapper.RatingMapper;
import com.studyback.hospitalservicesystem.service.RatingService;
import org.springframework.stereotype.Service;

/**
 * 满意度评价服务实现
 */
@Service
public class RatingServiceImpl extends ServiceImpl<RatingMapper, Rating> implements RatingService {

    @Override
    public Page<Rating> pageWithCondition(Integer pageNum, Integer pageSize, Long userId, Integer minScore) {
        Page<Rating> page = new Page<>(pageNum, pageSize);
        LambdaQueryWrapper<Rating> wrapper = new LambdaQueryWrapper<>();

        if (userId != null) {
            wrapper.eq(Rating::getUserId, userId);
        }
        if (minScore != null) {
            wrapper.ge(Rating::getScore, minScore);
        }
        wrapper.orderByDesc(Rating::getCreateTime);

        return this.page(page, wrapper);
    }

    @Override
    public Rating getByIdOrNull(Long id) {
        return this.getById(id);
    }

}
