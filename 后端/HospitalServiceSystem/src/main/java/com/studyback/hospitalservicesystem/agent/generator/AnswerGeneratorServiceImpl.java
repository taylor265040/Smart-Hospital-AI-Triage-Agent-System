package com.studyback.hospitalservicesystem.agent.generator;

import com.studyback.hospitalservicesystem.dto.QAResponse.DepartmentRecommendation;
import com.studyback.hospitalservicesystem.entity.Department;
import com.studyback.hospitalservicesystem.service.DepartmentService;
import dev.langchain4j.data.message.AiMessage;
import dev.langchain4j.data.message.SystemMessage;
import dev.langchain4j.data.message.UserMessage;
import dev.langchain4j.model.chat.ChatLanguageModel;
import dev.langchain4j.model.output.Response;
import jakarta.annotation.PostConstruct;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.Resource;
import org.springframework.stereotype.Service;
import org.springframework.util.StreamUtils;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Answer Generator 实现
 */
@Slf4j
@Service
@RequiredArgsConstructor
public class AnswerGeneratorServiceImpl implements AnswerGeneratorService {

    private final ChatLanguageModel chatLanguageModel;
    private final DepartmentService departmentService;

    @Value("classpath:prompts/generator-system-prompt.txt")
    private Resource generatorPromptResource;

    private String generatorSystemPrompt;

    /** 解析：**推荐科室：科室名称|推荐理由** */
    private static final Pattern DEPT_PATTERN =
            Pattern.compile("\\*\\*推荐科室：?([^|]+)\\|(.+?)\\*\\*");

    /** 科室名→ID 缓存 */
    private volatile Map<String, Long> deptNameToId;

    @PostConstruct
    private void loadPrompt() {
        try {
            this.generatorSystemPrompt = StreamUtils.copyToString(
                    generatorPromptResource.getInputStream(), StandardCharsets.UTF_8).trim();
        } catch (IOException e) {
            throw new IllegalStateException("加载 Generator Prompt 文件失败", e);
        }
    }

    @Override
    public GenerationResult generate(String question, String context, String triageReport) {
        // 构建消息
        List<dev.langchain4j.data.message.ChatMessage> messages = new ArrayList<>();
        messages.add(SystemMessage.from(generatorSystemPrompt));

        StringBuilder userMsg = new StringBuilder();
        userMsg.append("【用户问题】\n").append(question).append("\n\n");
        if (context != null && !context.isBlank()) {
            userMsg.append("【导诊上下文】\n").append(context).append("\n");
        }
        if (triageReport != null && !triageReport.isBlank()) {
            userMsg.append(triageReport).append("\n");
        }
        userMsg.append("请基于以上信息回答用户问题。");

        messages.add(UserMessage.from(userMsg.toString()));

        Response<AiMessage> response = chatLanguageModel.generate(messages);
        String rawAnswer = response.content().text();

        // 提取并清理科室推荐
        DepartmentRecommendation deptRec = extractDepartment(rawAnswer);
        String cleanAnswer = rawAnswer;
        if (deptRec != null) {
            cleanAnswer = rawAnswer.replaceAll("\\*\\*推荐科室：[^*]+\\*\\*[\\s]*", "").trim();
        }

        log.info("AnswerGenerator: {} chars, 推荐: {}",
                cleanAnswer.length(), deptRec != null ? deptRec.getDepartmentName() : "无");

        return new GenerationResult(cleanAnswer, deptRec);
    }

    private DepartmentRecommendation extractDepartment(String answer) {
        Matcher m = DEPT_PATTERN.matcher(answer);
        if (m.find()) {
            String name = m.group(1).trim();
            String reason = m.group(2).trim();
            Long deptId = lookupDeptId(name);
            if (deptId != null) {
                return DepartmentRecommendation.builder()
                        .departmentId(deptId)
                        .departmentName(name)
                        .reason(reason)
                        .build();
            }
        }
        return null;
    }

    private Long lookupDeptId(String name) {
        Map<String, Long> map = getDeptNameToId();
        return map.getOrDefault(name, map.entrySet().stream()
                .filter(e -> e.getKey().contains(name) || name.contains(e.getKey()))
                .map(Map.Entry::getValue)
                .findFirst()
                .orElse(null));
    }

    private Map<String, Long> getDeptNameToId() {
        if (deptNameToId == null) {
            synchronized (this) {
                if (deptNameToId == null) {
                    Map<String, Long> map = new LinkedHashMap<>();
                    List<Department> depts = departmentService.listEnabled();
                    for (Department d : depts) {
                        map.put(d.getName(), d.getId());
                    }
                    deptNameToId = map;
                }
            }
        }
        return deptNameToId;
    }
}
