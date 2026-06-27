package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.studyback.hospitalservicesystem.entity.Conversation;
import com.studyback.hospitalservicesystem.mapper.ConversationMapper;
import com.studyback.hospitalservicesystem.service.ConversationService;
import org.springframework.stereotype.Service;

/**
 * 会话服务实现
 */
@Service
public class ConversationServiceImpl extends ServiceImpl<ConversationMapper, Conversation> implements ConversationService {

    @Override
    public Page<Conversation> pageWithCondition(Integer pageNum, Integer pageSize, Long userId, String type, String status) {
        Page<Conversation> page = new Page<>(pageNum, pageSize);
        LambdaQueryWrapper<Conversation> wrapper = new LambdaQueryWrapper<>();

        if (userId != null) {
            wrapper.eq(Conversation::getUserId, userId);
        }
        if (type != null) {
            wrapper.eq(Conversation::getType, type);
        }
        if (status != null) {
            wrapper.eq(Conversation::getStatus, status);
        }
        wrapper.orderByDesc(Conversation::getCreateTime);

        return this.page(page, wrapper);
    }

    @Override
    public Conversation getByIdOrNull(Long id) {
        return this.getById(id);
    }

}
