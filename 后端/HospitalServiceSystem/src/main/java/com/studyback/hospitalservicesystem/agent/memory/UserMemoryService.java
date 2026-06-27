package com.studyback.hospitalservicesystem.agent.memory;

import java.util.Map;

/**
 * 用户长期记忆服务
 * <p>
 * 持久化用户偏好、病史、上次就诊信息等 KV 数据。
 */
public interface UserMemoryService {

    /**
     * 记录一条用户记忆
     *
     * @param userId 用户ID
     * @param key    记忆键
     * @param value  记忆值
     */
    void remember(Long userId, String key, String value);

    /**
     * 召回用户的所有长期记忆
     *
     * @param userId 用户ID
     * @return 记忆键值对
     */
    Map<String, String> recall(Long userId);

    /**
     * 删除某条记忆
     */
    void forget(Long userId, String key);

    /**
     * 获取格式化的记忆摘要（供 Agent Prompt 使用）
     */
    String recallAsText(Long userId);

    /**
     * 清理过期记忆
     */
    int cleanExpired();
}
