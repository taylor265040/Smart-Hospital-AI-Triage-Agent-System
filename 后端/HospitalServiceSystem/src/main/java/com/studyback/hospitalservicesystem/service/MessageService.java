package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.entity.Message;

import java.util.List;

/**
 * 消息服务接口
 */
public interface MessageService extends IService<Message> {

    /**
     * 分页条件查询消息
     */
    Page<Message> pageWithCondition(Integer pageNum, Integer pageSize, Long conversationId, String senderType);

    /**
     * 查询指定会话的所有消息（按时间正序）
     */
    List<Message> listByConversationId(Long conversationId);

    /**
     * 获取会话最近 N 条消息（按时间正序）
     * 供 ChatMemory 从 DB 加载历史对话
     */
    List<Message> getRecentByConversationId(Long conversationId, int limit);

    /**
     * 根据ID查询消息，不存在返回null
     */
    Message getByIdOrNull(Long id);

}
