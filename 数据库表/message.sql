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

 Date: 25/06/2026 12:46:14
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for message
-- ----------------------------
DROP TABLE IF EXISTS `message`;
CREATE TABLE `message`  (
  `id` bigint(20) NOT NULL COMMENT '主键ID',
  `conversation_id` bigint(20) NOT NULL COMMENT '所属会话ID',
  `sender_id` bigint(20) NOT NULL COMMENT '发送者ID',
  `sender_type` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '发送者类型',
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '消息内容',
  `message_type` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL DEFAULT 'TEXT' COMMENT '消息类型',
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT '逻辑删除',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `idx_conversation_id`(`conversation_id` ASC) USING BTREE,
  INDEX `idx_sender_id`(`sender_id` ASC) USING BTREE,
  INDEX `idx_create_time`(`create_time` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '消息表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of message
-- ----------------------------
INSERT INTO `message` VALUES (1001, 100, 2, 'USER', '你好，我咳嗽好几天了', 'TEXT', 0, '2026-06-25 08:00:00');
INSERT INTO `message` VALUES (1002, 100, 0, 'AI', '您好！咳嗽好几天确实挺难受的。能告诉我还有其他症状吗？比如有没有发烧、咳痰、胸闷等情况？', 'TEXT', 0, '2026-06-25 08:00:05');
INSERT INTO `message` VALUES (1003, 100, 2, 'USER', '有点低烧，还有黄痰', 'TEXT', 0, '2026-06-25 08:00:30');
INSERT INTO `message` VALUES (1004, 100, 0, 'AI', '根据您的描述（咳嗽、低烧、黄痰），可能是呼吸道感染。建议您挂呼吸内科（门诊楼3层东侧A区），让医生帮您做个详细检查。', 'TEXT', 0, '2026-06-25 08:00:35');
INSERT INTO `message` VALUES (1005, 101, 3, 'USER', '帮我查下心内科有哪些医生', 'TEXT', 0, '2026-06-25 09:00:00');
INSERT INTO `message` VALUES (1006, 101, 0, 'AI', '心内科有以下在岗医生：张伟主任医师（擅长冠心病、高血压、心律失常）、刘洋副主任医师（擅长冠脉介入治疗）。请问您想了解哪位医生？', 'TEXT', 0, '2026-06-25 09:00:05');

SET FOREIGN_KEY_CHECKS = 1;
