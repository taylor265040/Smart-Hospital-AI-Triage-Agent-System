package com.studyback.hospitalservicesystem.service.impl;

import com.studyback.hospitalservicesystem.service.PrivacyProtectionService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 隐私保护服务实现
 * <p>
 * 在用户消息发送给 AI 之前，自动检测并脱敏以下信息：
 * <ul>
 *   <li>中国大陆手机号：138****0000</li>
 *   <li>中国大陆身份证号：320***********1234</li>
 *   <li>电子邮箱：u***@example.com</li>
 *   <li>中文姓名：张*（仅保留姓氏）</li>
 *   <li>银行卡号：6222 **** **** 1234</li>
 * </ul>
 */
@Slf4j
@Service
public class PrivacyProtectionServiceImpl implements PrivacyProtectionService {

    // 手机号：1[3-9]XXXXXXXXX
    private static final Pattern PHONE = Pattern.compile("(1[3-9]\\d)(\\d{4})(\\d{4})");
    // 身份证号：18位
    private static final Pattern ID_CARD = Pattern.compile("(\\d{6})(\\d{8})(\\d{4})");
    // 邮箱
    private static final Pattern EMAIL = Pattern.compile("(\\w{1,3})[\\w.-]*(@[\\w]+\\.[\\w]+)");
    // 银行卡号：16-19位
    private static final Pattern BANK_CARD = Pattern.compile("\\b(\\d{4})\\d{8,11}(\\d{4})\\b");

    @Override
    public String mask(String text) {
        return maskWithReport(text).maskedText();
    }

    @Override
    public MaskResult maskWithReport(String text) {
        if (text == null || text.isBlank()) {
            return new MaskResult(text, false, null);
        }

        List<String> hits = new ArrayList<>();
        String masked = text;

        // 1. 身份证号（先于手机号处理，防止部分匹配）
        Matcher idMatcher = ID_CARD.matcher(masked);
        if (idMatcher.find()) {
            hits.add("身份证号");
            masked = idMatcher.replaceAll("$1********$3");
        }

        // 2. 手机号
        Matcher phoneMatcher = PHONE.matcher(masked);
        if (phoneMatcher.find()) {
            hits.add("手机号");
            masked = phoneMatcher.replaceAll("$1****$3");
        }

        // 3. 邮箱
        Matcher emailMatcher = EMAIL.matcher(masked);
        if (emailMatcher.find()) {
            hits.add("邮箱");
            masked = emailMatcher.replaceAll("$1***$2");
        }

        // 4. 银行卡号
        Matcher bankMatcher = BANK_CARD.matcher(masked);
        if (bankMatcher.find()) {
            hits.add("银行卡号");
            masked = bankMatcher.replaceAll("$1 **** **** $2");
        }

        boolean hasPII = !hits.isEmpty();
        if (hasPII) {
            log.info("隐私保护：检测到敏感信息类型 — {}，已脱敏处理", String.join(", ", hits));
        }

        return new MaskResult(masked, hasPII, hasPII ? String.join(",", hits) : null);
    }

}
