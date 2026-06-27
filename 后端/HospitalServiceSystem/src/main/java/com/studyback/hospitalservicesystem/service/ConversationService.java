package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.entity.Conversation;

/**
 * 会话服务接口
 */
public interface ConversationService extends IService<Conversation> {

    /**
     * 分页条件查询会话
     */
    Page<Conversation> pageWithCondition(Integer pageNum, Integer pageSize, Long userId, String type, String status);

    /**
     * 根据ID查询会话，不存在返回null
     */
    Conversation getByIdOrNull(Long id);

}
