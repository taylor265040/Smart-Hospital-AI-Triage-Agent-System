/*
 Navicat Premium Data Transfer

 Source Server         : root
 Source Server Type    : MySQL
 Source Server Version : 80018 (8.0.18)
 Source Host           : localhost:3306
 Source Schema         : hospitalservicesystem

 Target Server Type    : MySQL
 Target Server Version : 80018 (8.0.18)
 File Encoding         : 65001

 Date: 25/06/2026 12:46:25
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for user
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user`  (
  `id` bigint(20) NOT NULL COMMENT '主键ID',
  `username` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '用户名',
  `password` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '密码（加密存储）',
  `real_name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '真实姓名',
  `phone` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '手机号',
  `email` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '邮箱',
  `role` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL DEFAULT 'PATIENT' COMMENT '角色',
  `avatar` varchar(500) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '头像URL',
  `status` int(11) NOT NULL DEFAULT 1 COMMENT '状态：1-启用 0-禁用',
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT '逻辑删除',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  `is_online` int(11) NOT NULL DEFAULT 0 COMMENT '客服在线状态：1=在线, 0=离线（仅CS_STAFF使用）',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `uk_username`(`username` ASC) USING BTREE,
  INDEX `idx_role`(`role` ASC) USING BTREE,
  INDEX `idx_phone`(`phone` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '用户表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of user
-- ----------------------------
INSERT INTO `user` VALUES (1, 'admin', 'e10adc3949ba59abbe56e057f20f883e', '系统管理员', '13800000000', 'admin@hospital.com', 'ADMIN', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (2, 'zhangsan', 'e10adc3949ba59abbe56e057f20f883e', '张三', '13800000001', 'zhangsan@test.com', 'PATIENT', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (3, 'lisi', 'e10adc3949ba59abbe56e057f20f883e', '李四', '13800000002', 'lisi@test.com', 'PATIENT', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (4, 'wangwu', 'e10adc3949ba59abbe56e057f20f883e', '王五', '13800000003', 'wangwu@test.com', 'PATIENT', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (5, 'cs_zhang', 'e10adc3949ba59abbe56e057f20f883e', '张客服', '13800000010', 'cs_zhang@hospital.com', 'CS_STAFF', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 1);
INSERT INTO `user` VALUES (6, 'cs_li', 'e10adc3949ba59abbe56e057f20f883e', '李客服', '13800000011', 'cs_li@hospital.com', 'CS_STAFF', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 1);
INSERT INTO `user` VALUES (7, 'doctor_zw', 'e10adc3949ba59abbe56e057f20f883e', '张伟', '13800000101', 'zhangwei@hospital.com', 'DOCTOR', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (8, 'doctor_wf', 'e10adc3949ba59abbe56e057f20f883e', '王芳', '13800000102', 'wangfang@hospital.com', 'DOCTOR', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (9, 'doctor_lm', 'e10adc3949ba59abbe56e057f20f883e', '李明', '13800000103', 'liming@hospital.com', 'DOCTOR', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (10, 'doctor_cq', 'e10adc3949ba59abbe56e057f20f883e', '陈强', '13800000110', 'chenqiang@hospital.com', 'DOCTOR', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (11, 'doctor_sl', 'e10adc3949ba59abbe56e057f20f883e', '孙丽', '13800000113', 'sunli@hospital.com', 'DOCTOR', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (12, 'doctor_wm', 'e10adc3949ba59abbe56e057f20f883e', '吴敏', '13800000115', 'wumin@hospital.com', 'DOCTOR', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (13, 'doctor_zh', 'e10adc3949ba59abbe56e057f20f883e', '郑华', '13800000116', 'zhenghua@hospital.com', 'DOCTOR', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);
INSERT INTO `user` VALUES (14, 'doctor_yg', 'e10adc3949ba59abbe56e057f20f883e', '杨刚', '13800000119', 'yanggang@hospital.com', 'DOCTOR', NULL, 1, 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34', 0);

SET FOREIGN_KEY_CHECKS = 1;
