package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.studyback.hospitalservicesystem.entity.Announcement;
import com.studyback.hospitalservicesystem.mapper.AnnouncementMapper;
import com.studyback.hospitalservicesystem.service.AnnouncementService;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import java.time.LocalDateTime;
import java.util.List;

/**
 * 公告服务实现
 */
@Service
public class AnnouncementServiceImpl extends ServiceImpl<AnnouncementMapper, Announcement> implements AnnouncementService {

    @Override
    public Page<Announcement> pageWithCondition(Integer pageNum, Integer pageSize, String keyword, String type, Integer status) {
        Page<Announcement> page = new Page<>(pageNum, pageSize);
        LambdaQueryWrapper<Announcement> wrapper = new LambdaQueryWrapper<>();

        if (StringUtils.hasText(keyword)) {
            wrapper.like(Announcement::getTitle, keyword);
        }
        if (StringUtils.hasText(type)) {
            wrapper.eq(Announcement::getType, type);
        }
        if (status != null) {
            wrapper.eq(Announcement::getStatus, status);
        }
        wrapper.orderByDesc(Announcement::getCreateTime);

        return this.page(page, wrapper);
    }

    @Override
    public List<Announcement> listPublished() {
        LambdaQueryWrapper<Announcement> wrapper = new LambdaQueryWrapper<>();
        wrapper.eq(Announcement::getStatus, 1)
                .orderByDesc(Announcement::getPublishTime);
        return this.list(wrapper);
    }

    @Override
    public Announcement getByIdOrNull(Long id) {
        return this.getById(id);
    }

    @Override
    public void publish(Long id) {
        Announcement announcement = this.getById(id);
        if (announcement != null) {
            announcement.setStatus(1);
            announcement.setPublishTime(LocalDateTime.now());
            this.updateById(announcement);
        }
    }

}
