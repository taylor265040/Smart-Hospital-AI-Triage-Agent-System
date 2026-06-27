package com.studyback.hospitalservicesystem.service;

import com.studyback.hospitalservicesystem.dto.HumanServiceResult;

import java.util.List;
import java.util.Map;

/**
 * 人工客服队列服务
 * <p>
 * 核心流程：
 * <ol>
 *   <li>用户请求人工 → 入队（conversation status=QUEUED）</li>
 *   <li>CS 上线 → 自动从队列中分配等待用户</li>
 *   <li>CS 主动接受 → 分配指定用户</li>
 *   <li>对话结束 → CS 关闭会话</li>
 * </ol>
 */
public interface HumanAgentService {

    // ==================== 用户端 ====================

    /**
     * 用户请求人工客服（加入排队队列）
     *
     * @param userId 用户ID
     * @param reason 请求原因（可选，如"AI无法回答"）
     * @return 排队结果
     */
    HumanServiceResult requestHumanService(Long userId, String reason);

    /**
     * 查询用户排队状态
     *
     * @param userId 用户ID
     * @return 排队信息
     */
    HumanServiceResult checkQueueStatus(Long userId);

    /**
     * 用户取消排队
     *
     * @param userId 用户ID
     */
    void cancelQueue(Long userId);

    // ==================== 客服端 ====================

    /**
     * 客服上线
     */
    void staffOnline(Long csStaffId);

    /**
     * 客服下线
     */
    void staffOffline(Long csStaffId);

    /**
     * 获取当前排队列表（供客服端查看）
     */
    List<Map<String, Object>> getPendingQueue();

    /**
     * 客服接受某个排队用户
     *
     * @param csStaffId    客服ID
     * @param conversationId 排队会话ID
     * @return 接管结果
     */
    HumanServiceResult acceptUser(Long csStaffId, Long conversationId);

    /**
     * 客服关闭人工服务会话
     *
     * @param csStaffId     客服ID
     * @param conversationId 会话ID
     */
    void closeSession(Long csStaffId, Long conversationId);

    /**
     * 获取在线客服列表
     */
    List<Map<String, Object>> getOnlineStaff();

}
