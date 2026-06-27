package com.studyback.hospitalservicesystem.agent.memory;

import java.util.List;
import java.util.Map;

/**
 * 短期对话记忆服务
 * <p>
 * 维护最近 N 轮对话（默认 20 轮），持久化到 message 表。
 */
public interface ChatMemoryService {

    /**
     * 添加一条消息到记忆
     *
     * @param conversationId 会话ID
     * @param role           角色（user / assistant）
     * @param content        消息内容
     */
    void addMessage(Long conversationId, String role, String content);

    /**
     * 获取最近 N 轮对话（格式化文本，供 Agent Prompt 使用）
     *
     * @param conversationId 会话ID
     * @param maxRounds      最大轮数
     * @return 格式化的对话文本
     */
    String getRecentMessagesText(Long conversationId, int maxRounds);

    /**
     * 获取最近的消息列表（结构化）
     *
     * @param conversationId 会话ID
     * @param maxMessages    最大消息数
     * @return 消息列表，每条含 role/content
     */
    List<Map<String, String>> getRecentMessages(Long conversationId, int maxMessages);

    /**
     * 清空会话记忆
     */
    void clear(Long conversationId);
}
