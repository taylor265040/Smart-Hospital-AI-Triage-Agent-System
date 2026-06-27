package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.entity.Announcement;

import java.util.List;

/**
 * 公告服务接口
 */
public interface AnnouncementService extends IService<Announcement> {

    /**
     * 分页条件查询公告
     */
    Page<Announcement> pageWithCondition(Integer pageNum, Integer pageSize, String keyword, String type, Integer status);

    /**
     * 查询所有已发布的公告（患者端用）
     */
    List<Announcement> listPublished();

    /**
     * 根据ID查询公告，不存在返回null
     */
    Announcement getByIdOrNull(Long id);

    /**
     * 发布公告
     */
    void publish(Long id);

}
