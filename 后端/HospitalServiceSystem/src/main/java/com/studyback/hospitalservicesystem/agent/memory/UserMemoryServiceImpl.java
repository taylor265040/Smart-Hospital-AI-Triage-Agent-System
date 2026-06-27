package com.studyback.hospitalservicesystem.agent.memory;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.studyback.hospitalservicesystem.entity.UserMemory;
import com.studyback.hospitalservicesystem.mapper.UserMemoryMapper;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.dao.DuplicateKeyException;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * 用户长期记忆实现
 */
@Slf4j
@Service
@RequiredArgsConstructor
public class UserMemoryServiceImpl implements UserMemoryService {

    private final UserMemoryMapper userMemoryMapper;

    @Override
    public void remember(Long userId, String key, String value) {
        // 先尝试更新（大多数情况下 Key 已存在）
        UserMemory exist = userMemoryMapper.selectOne(
                new LambdaQueryWrapper<UserMemory>()
                        .eq(UserMemory::getUserId, userId)
                        .eq(UserMemory::getMemoryKey, key)
                        .last("LIMIT 1")
        );

        if (exist != null) {
            exist.setMemoryValue(value);
            exist.setUpdateTime(LocalDateTime.now());
            userMemoryMapper.updateById(exist);
            log.debug("UserMemory updated: userId={}, key={}", userId, key);
            return;
        }

        // 不存在则插入，捕获唯一约束冲突（并发安全 + 逻辑删除回退）
        try {
            UserMemory mem = new UserMemory();
            mem.setUserId(userId);
            mem.setMemoryKey(key);
            mem.setMemoryValue(value);
            userMemoryMapper.insert(mem);
            log.debug("UserMemory inserted: userId={}, key={}", userId, key);
        } catch (DuplicateKeyException e) {
            // 并发冲突或逻辑删除回退：转为更新
            log.debug("UserMemory duplicate key, fallback to update: userId={}, key={}", userId, key);
            exist = userMemoryMapper.selectOne(
                    new LambdaQueryWrapper<UserMemory>()
                            .eq(UserMemory::getUserId, userId)
                            .eq(UserMemory::getMemoryKey, key)
                            .last("LIMIT 1")
            );
            if (exist != null) {
                exist.setMemoryValue(value);
                exist.setUpdateTime(LocalDateTime.now());
                userMemoryMapper.updateById(exist);
            }
        }
    }

    @Override
    public Map<String, String> recall(Long userId) {
        List<UserMemory> memories = userMemoryMapper.selectList(
                new LambdaQueryWrapper<UserMemory>()
                        .eq(UserMemory::getUserId, userId)
                        .and(w -> w.isNull(UserMemory::getExpireTime)
                                .or().gt(UserMemory::getExpireTime, LocalDateTime.now()))
        );

        Map<String, String> result = new LinkedHashMap<>();
        for (UserMemory m : memories) {
            result.put(m.getMemoryKey(), m.getMemoryValue());
        }
        return result;
    }

    @Override
    public void forget(Long userId, String key) {
        // 物理删除，避免唯一约束冲突
        userMemoryMapper.delete(new LambdaQueryWrapper<UserMemory>()
                .eq(UserMemory::getUserId, userId)
                .eq(UserMemory::getMemoryKey, key));
    }

    @Override
    public String recallAsText(Long userId) {
        Map<String, String> memory = recall(userId);
        if (memory.isEmpty()) {
            return "";
        }

        StringBuilder sb = new StringBuilder();
        for (Map.Entry<String, String> entry : memory.entrySet()) {
            String label = switch (entry.getKey()) {
                case "medical_history" -> "病史";
                case "preferred_doctor" -> "偏好医生";
                case "last_department" -> "上次就诊科室";
                case "last_appointment" -> "上次预约";
                case "last_interaction" -> "上次交互时间";
                default -> entry.getKey();
            };
            sb.append(label).append(": ").append(entry.getValue()).append("\n");
        }
        return sb.toString();
    }

    @Override
    public int cleanExpired() {
        int deleted = userMemoryMapper.delete(new LambdaQueryWrapper<UserMemory>()
                .lt(UserMemory::getExpireTime, LocalDateTime.now()));
        if (deleted > 0) {
            log.info("清理了 {} 条过期用户记忆", deleted);
        }
        return deleted;
    }
}
