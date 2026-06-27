package com.studyback.hospitalservicesystem.service;

/**
 * 隐私保护服务 — 对用户输入进行敏感信息脱敏
 */
public interface PrivacyProtectionService {

    /**
     * 对文本中的敏感信息进行脱敏替换
     *
     * @param text 原始文本
     * @return 脱敏后的文本
     */
    String mask(String text);

    /**
     * 脱敏并返回命中信息（用于日志记录）
     *
     * @param text 原始文本
     * @return 脱敏结果 + 检测报告
     */
    MaskResult maskWithReport(String text);

    /**
     * 脱敏结果
     */
    record MaskResult(String maskedText, boolean hasPII, String piiTypes) {}

}
