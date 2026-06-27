package com.studyback.hospitalservicesystem.agent.memory;

import com.studyback.hospitalservicesystem.entity.Message;
import com.studyback.hospitalservicesystem.service.MessageService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 短期对话记忆实现
 * <p>
 * 使用内存缓存 + DB 持久化的双层存储：
 * - 内存缓存：加速当前会话的读写（MessageWindowChatMemory 语义）
 * - DB 持久化：消息通过 MessageService 保存到 message 表
 */
@Slf4j
@Service
@RequiredArgsConstructor
public class ChatMemoryServiceImpl implements ChatMemoryService {

    private final MessageService messageService;

    /** 会话ID → 最近消息列表（内存缓存） */
    private final Map<Long, List<Map<String, String>>> memoryCache = new ConcurrentHashMap<>();

    @Override
    public void addMessage(Long conversationId, String role, String content) {
        // 1. 写入内存缓存
        List<Map<String, String>> messages = memoryCache.computeIfAbsent(conversationId,
                k -> Collections.synchronizedList(new ArrayList<>()));

        Map<String, String> msg = new LinkedHashMap<>();
        msg.put("role", role);
        msg.put("content", content);
        messages.add(msg);

        // 限制内存中的消息数
        int maxMessages = 40; // 20 轮
        while (messages.size() > maxMessages) {
            messages.remove(0);
        }

        // 2. 持久化到 DB（去重：检查最近一条消息是否相同）
        try {
            List<Message> recent = messageService.getRecentByConversationId(conversationId, 1);
            boolean isDuplicate = !recent.isEmpty()
                    && recent.get(0).getContent() != null
                    && recent.get(0).getContent().equals(content)
                    && recent.get(0).getSenderType() != null
                    && recent.get(0).getSenderType().equalsIgnoreCase(role);

            if (!isDuplicate) {
                Message dbMsg = new Message();
                dbMsg.setConversationId(conversationId);
                dbMsg.setSenderType(role.toUpperCase());
                dbMsg.setContent(content);
                dbMsg.setSenderId("user".equalsIgnoreCase(role) ? 0L : 0L); // senderId from context if available
                messageService.save(dbMsg);
            }
        } catch (Exception e) {
            log.warn("ChatMemory 持久化到 DB 失败: convId={}", conversationId, e);
        }
    }

    @Override
    public String getRecentMessagesText(Long conversationId, int maxRounds) {
        List<Map<String, String>> messages = getRecentMessages(conversationId, maxRounds * 2);
        if (messages.isEmpty()) {
            return "";
        }

        StringBuilder sb = new StringBuilder();
        for (Map<String, String> msg : messages) {
            String roleLabel = switch (msg.getOrDefault("role", "")) {
                case "user" -> "用户";
                case "assistant" -> "AI助手";
                default -> msg.get("role");
            };
            sb.append(roleLabel).append("：").append(msg.get("content")).append("\n");
        }
        return sb.toString();
    }

    @Override
    public List<Map<String, String>> getRecentMessages(Long conversationId, int maxMessages) {
        // 优先从内存缓存读取
        List<Map<String, String>> cached = memoryCache.get(conversationId);
        if (cached != null && !cached.isEmpty()) {
            synchronized (cached) {
                int from = Math.max(0, cached.size() - maxMessages);
                return new ArrayList<>(cached.subList(from, cached.size()));
            }
        }

        // 回退到 DB
        try {
            List<Message> dbMessages = messageService.getRecentByConversationId(conversationId, maxMessages);
            List<Map<String, String>> result = new ArrayList<>();
            for (Message m : dbMessages) {
                Map<String, String> msg = new LinkedHashMap<>();
                msg.put("role", m.getSenderType() != null ? m.getSenderType().toLowerCase() : "unknown");
                msg.put("content", m.getContent());
                result.add(msg);
            }
            // 回填到内存缓存
            if (!result.isEmpty()) {
                memoryCache.put(conversationId, Collections.synchronizedList(new ArrayList<>(result)));
            }
            return result;
        } catch (Exception e) {
            log.warn("从 DB 加载会话 {} 记忆失败", conversationId, e);
            return List.of();
        }
    }

    @Override
    public void clear(Long conversationId) {
        memoryCache.remove(conversationId);
    }
}
