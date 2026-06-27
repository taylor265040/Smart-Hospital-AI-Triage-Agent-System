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

 Date: 25/06/2026 12:45:39
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for announcement
-- ----------------------------
DROP TABLE IF EXISTS `announcement`;
CREATE TABLE `announcement`  (
  `id` bigint(20) NOT NULL COMMENT '主键ID',
  `title` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '公告标题',
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '公告内容',
  `type` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL DEFAULT 'NOTICE' COMMENT '公告类型',
  `status` int(11) NOT NULL DEFAULT 0 COMMENT '状态',
  `publisher_id` bigint(20) NULL DEFAULT NULL COMMENT '发布人ID',
  `publish_time` datetime NULL DEFAULT NULL COMMENT '发布时间',
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT '逻辑删除',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `idx_type`(`type` ASC) USING BTREE,
  INDEX `idx_status`(`status` ASC) USING BTREE,
  INDEX `idx_publish_time`(`publish_time` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '公告表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of announcement
-- ----------------------------
INSERT INTO `announcement` VALUES (1, '关于端午节期间门诊安排的通知', '根据国家法定节假日安排，6月25日（端午节）门诊停诊一天，急诊科、发热门诊24小时正常开放。6月26日起恢复正常门诊。请各位患者合理安排就诊时间。', 'NOTICE', 1, NULL, '2026-06-20 08:00:00', 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34');
INSERT INTO `announcement` VALUES (2, '我院通过三级甲等医院复审', '喜讯！我院于2026年6月顺利通过国家卫健委三级甲等医院复审。感谢全体医护人员和广大患者的支持与信任。', 'ACTIVITY', 1, NULL, '2026-06-15 10:00:00', 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34');
INSERT INTO `announcement` VALUES (3, '健康讲座：夏季肠胃疾病预防', '我院消化内科将于7月2日（周六）上午9:30在门诊楼5层学术报告厅举办\"夏季肠胃疾病预防\"健康讲座，由孙丽副主任医师主讲，欢迎广大市民免费参加。', 'ACTIVITY', 1, NULL, '2026-06-22 09:00:00', 0, '2026-06-25 12:40:34', '2026-06-25 12:40:34');

SET FOREIGN_KEY_CHECKS = 1;
