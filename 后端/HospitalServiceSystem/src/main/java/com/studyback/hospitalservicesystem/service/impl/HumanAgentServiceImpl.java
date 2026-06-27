package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.studyback.hospitalservicesystem.dto.HumanServiceResult;
import com.studyback.hospitalservicesystem.entity.Conversation;
import com.studyback.hospitalservicesystem.entity.User;
import com.studyback.hospitalservicesystem.service.ConversationService;
import com.studyback.hospitalservicesystem.service.HumanAgentService;
import com.studyback.hospitalservicesystem.service.UserService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.*;
import java.util.stream.Collectors;

/**
 * 人工客服队列服务实现
 */
@Slf4j
@Service
@RequiredArgsConstructor
public class HumanAgentServiceImpl implements HumanAgentService {

    private final ConversationService conversationService;
    private final UserService userService;

    // ==================== 用户端 ====================

    @Override
    @Transactional
    public HumanServiceResult requestHumanService(Long userId, String reason) {
        // 1. 检查是否已在排队
        long alreadyInQueue = conversationService.count(
                new LambdaQueryWrapper<Conversation>()
                        .eq(Conversation::getUserId, userId)
                        .eq(Conversation::getStatus, "QUEUED")
        );
        if (alreadyInQueue > 0) {
            int position = getQueuePosition(userId);
            return HumanServiceResult.builder()
                    .success(true)
                    .message("您已在排队中")
                    .queuePosition(position)
                    .waitingCount(position - 1)
                    .build();
        }

        // 2. 创建排队会话（始终进入队列，等待客服主动接入）
        Conversation conv = new Conversation();
        conv.setUserId(userId);
        conv.setType("MANUAL");
        conv.setStatus("QUEUED");
        conversationService.save(conv);

        int position = getQueuePosition(userId);
        log.info("用户 {} 加入人工客服队列，当前排队位置: {}", userId, position);
        return HumanServiceResult.builder()
                .success(true)
                .message("您已进入人工客服排队队列，请耐心等待")
                .queuePosition(position)
                .waitingCount(position - 1)
                .build();
    }

    @Override
    public HumanServiceResult checkQueueStatus(Long userId) {
        int position = getQueuePosition(userId);
        if (position <= 0) {
            // 检查是否有活跃的人工会话
            Conversation active = getActiveHumanSession(userId);
            if (active != null) {
                User staff = userService.getById(active.getCsStaffId());
                return HumanServiceResult.builder()
                        .success(true)
                        .message("人工客服已接入")
                        .queuePosition(0)
                        .waitingCount(0)
                        .csStaffName(staff != null ? staff.getRealName() : "客服")
                        .conversationId(active.getId())
                        .build();
            }
            return HumanServiceResult.builder()
                    .success(true)
                    .message("您当前不在排队中")
                    .queuePosition(0)
                    .waitingCount(0)
                    .build();
        }

        return HumanServiceResult.builder()
                .success(true)
                .message("排队中")
                .queuePosition(position)
                .waitingCount(position - 1)
                .build();
    }

    @Override
    public void cancelQueue(Long userId) {
        List<Conversation> queued = conversationService.list(
                new LambdaQueryWrapper<Conversation>()
                        .eq(Conversation::getUserId, userId)
                        .eq(Conversation::getStatus, "QUEUED")
        );
        for (Conversation conv : queued) {
            conv.setStatus("CLOSED");
            conversationService.updateById(conv);
        }
        log.info("用户 {} 已取消排队", userId);
    }

    // ==================== 客服端 ====================

    @Override
    public void staffOnline(Long csStaffId) {
        User staff = userService.getById(csStaffId);
        if (staff == null || !"CS_STAFF".equals(staff.getRole())) {
            throw new IllegalArgumentException("非法的客服人员ID");
        }
        staff.setIsOnline(1);
        userService.updateById(staff);
        log.info("客服 {} ({}) 上线，等待手动接入用户", csStaffId, staff.getRealName());
    }

    @Override
    public void staffOffline(Long csStaffId) {
        User staff = new User();
        staff.setId(csStaffId);
        staff.setIsOnline(0);
        userService.updateById(staff);
        log.info("客服 {} 下线", csStaffId);
    }

    @Override
    public List<Map<String, Object>> getPendingQueue() {
        List<Conversation> queued = conversationService.list(
                new LambdaQueryWrapper<Conversation>()
                        .eq(Conversation::getStatus, "QUEUED")
                        .orderByAsc(Conversation::getCreateTime)
        );

        return queued.stream().map(conv -> {
            User user = userService.getById(conv.getUserId());
            Map<String, Object> item = new LinkedHashMap<>();
            item.put("conversationId", conv.getId());
            item.put("userId", conv.getUserId());
            item.put("userName", user != null ? user.getRealName() : "未知");
            item.put("createTime", conv.getCreateTime().toString());
            int pos = getQueuePosition(conv.getUserId());
            item.put("queuePosition", pos);
            return item;
        }).collect(Collectors.toList());
    }

    @Override
    @Transactional
    public HumanServiceResult acceptUser(Long csStaffId, Long conversationId) {
        Conversation conv = conversationService.getById(conversationId);
        if (conv == null || !"QUEUED".equals(conv.getStatus())) {
            return HumanServiceResult.builder().success(false).message("该会话不在排队中").build();
        }

        conv.setCsStaffId(csStaffId);
        conv.setStatus("ACTIVE");
        conversationService.updateById(conv);

        User user = userService.getById(conv.getUserId());
        User staff = userService.getById(csStaffId);

        log.info("客服 {} 已接受用户 {} 的人工服务请求", csStaffId, conv.getUserId());

        return HumanServiceResult.builder()
                .success(true)
                .message("已接管会话")
                .queuePosition(0)
                .waitingCount(0)
                .csStaffName(staff != null ? staff.getRealName() : "客服")
                .conversationId(conv.getId())
                .build();
    }

    @Override
    public void closeSession(Long csStaffId, Long conversationId) {
        Conversation conv = conversationService.getById(conversationId);
        if (conv == null) {
            throw new IllegalArgumentException("会话不存在");
        }
        if (!csStaffId.equals(conv.getCsStaffId())) {
            throw new IllegalArgumentException("无权关闭他人会话");
        }
        conv.setStatus("CLOSED");
        conversationService.updateById(conv);
        log.info("客服 {} 已关闭会话 {}", csStaffId, conversationId);
    }

    @Override
    public List<Map<String, Object>> getOnlineStaff() {
        List<User> onlineStaff = userService.list(
                new LambdaQueryWrapper<User>()
                        .eq(User::getRole, "CS_STAFF")
                        .eq(User::getIsOnline, 1)
        );
        return onlineStaff.stream().map(staff -> {
            Map<String, Object> item = new LinkedHashMap<>();
            item.put("id", staff.getId());
            item.put("name", staff.getRealName());
            item.put("username", staff.getUsername());
            // 该客服当前服务的用户数
            long servingCount = conversationService.count(
                    new LambdaQueryWrapper<Conversation>()
                            .eq(Conversation::getCsStaffId, staff.getId())
                            .eq(Conversation::getStatus, "ACTIVE")
            );
            item.put("servingCount", servingCount);
            return item;
        }).collect(Collectors.toList());
    }

    // ==================== 私有方法 ====================

    /**
     * 获取用户排队位置（1-based）
     */
    private int getQueuePosition(Long userId) {
        List<Conversation> queued = conversationService.list(
                new LambdaQueryWrapper<Conversation>()
                        .eq(Conversation::getStatus, "QUEUED")
                        .orderByAsc(Conversation::getCreateTime)
        );
        for (int i = 0; i < queued.size(); i++) {
            if (queued.get(i).getUserId().equals(userId)) {
                return i + 1;
            }
        }
        return 0;
    }

    /**
     * 获取用户当前活跃的人工会话
     */
    private Conversation getActiveHumanSession(Long userId) {
        LambdaQueryWrapper<Conversation> wrapper = new LambdaQueryWrapper<Conversation>()
                .eq(Conversation::getUserId, userId)
                .eq(Conversation::getType, "MANUAL")
                .eq(Conversation::getStatus, "ACTIVE")
                .orderByDesc(Conversation::getCreateTime)
                .last("LIMIT 1");
        return conversationService.getOne(wrapper, false);
    }

    /**
     * 尝试自动分配：如果有在线且空闲的客服
     */
    private Conversation tryAutoAssign(Long conversationId) {
        List<User> onlineStaff = userService.list(
                new LambdaQueryWrapper<User>()
                        .eq(User::getRole, "CS_STAFF")
                        .eq(User::getIsOnline, 1)
        );
        if (onlineStaff.isEmpty()) return null;

        // 找当前服务人数最少的客服
        User bestStaff = null;
        long minLoad = Long.MAX_VALUE;
        for (User staff : onlineStaff) {
            long load = conversationService.count(
                    new LambdaQueryWrapper<Conversation>()
                            .eq(Conversation::getCsStaffId, staff.getId())
                            .eq(Conversation::getStatus, "ACTIVE")
            );
            if (load < minLoad) {
                minLoad = load;
                bestStaff = staff;
            }
        }

        if (bestStaff != null) {
            Conversation conv = conversationService.getById(conversationId);
            conv.setCsStaffId(bestStaff.getId());
            conv.setStatus("ACTIVE");
            conversationService.updateById(conv);
            log.info("自动分配：客服 {} → 用户会话 {}", bestStaff.getId(), conversationId);
            return conv;
        }
        return null;
    }

    /**
     * 客服上线时，自动从队列分配等待用户
     */
    private void autoAssignFromQueue(Long csStaffId) {
        List<Conversation> queued = conversationService.list(
                new LambdaQueryWrapper<Conversation>()
                        .eq(Conversation::getStatus, "QUEUED")
                        .orderByAsc(Conversation::getCreateTime)
        );
        if (!queued.isEmpty()) {
            Conversation first = queued.get(0);
            first.setCsStaffId(csStaffId);
            first.setStatus("ACTIVE");
            conversationService.updateById(first);
            log.info("自动分配：新上线客服 {} → 排队用户会话 {}", csStaffId, first.getId());
        }
    }

}
