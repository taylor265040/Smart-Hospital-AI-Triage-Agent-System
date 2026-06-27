package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.studyback.hospitalservicesystem.entity.Message;
import com.studyback.hospitalservicesystem.mapper.MessageMapper;
import com.studyback.hospitalservicesystem.service.MessageService;
import org.springframework.stereotype.Service;

import java.util.List;

/**
 * 消息服务实现
 */
@Service
public class MessageServiceImpl extends ServiceImpl<MessageMapper, Message> implements MessageService {

    @Override
    public Page<Message> pageWithCondition(Integer pageNum, Integer pageSize, Long conversationId, String senderType) {
        Page<Message> page = new Page<>(pageNum, pageSize);
        LambdaQueryWrapper<Message> wrapper = new LambdaQueryWrapper<>();

        if (conversationId != null) {
            wrapper.eq(Message::getConversationId, conversationId);
        }
        if (senderType != null) {
            wrapper.eq(Message::getSenderType, senderType);
        }
        wrapper.orderByAsc(Message::getCreateTime);

        return this.page(page, wrapper);
    }

    @Override
    public List<Message> listByConversationId(Long conversationId) {
        LambdaQueryWrapper<Message> wrapper = new LambdaQueryWrapper<>();
        wrapper.eq(Message::getConversationId, conversationId)
                .orderByAsc(Message::getCreateTime);
        return this.list(wrapper);
    }

    @Override
    public Message getByIdOrNull(Long id) {
        return this.getById(id);
    }

    @Override
    public List<Message> getRecentByConversationId(Long conversationId, int limit) {
        LambdaQueryWrapper<Message> wrapper = new LambdaQueryWrapper<>();
        wrapper.eq(Message::getConversationId, conversationId)
                .orderByDesc(Message::getCreateTime)
                .last("LIMIT " + limit);
        List<Message> messages = this.list(wrapper);
        // 反转为时间正序
        java.util.Collections.reverse(messages);
        return messages;
    }

}
