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

 Date: 25/06/2026 12:51:39
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for department
-- ----------------------------
DROP TABLE IF EXISTS `department`;
CREATE TABLE `department`  (
  `id` bigint(20) NOT NULL COMMENT '主键ID',
  `name` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '科室名称',
  `floor` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '所在楼层',
  `location` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '具体位置描述',
  `description` text CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL COMMENT '科室描述',
  `phone` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '科室电话',
  `sort_order` int(11) NOT NULL DEFAULT 0 COMMENT '排序',
  `status` int(11) NOT NULL DEFAULT 1 COMMENT '状态',
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT '逻辑删除',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `idx_name`(`name` ASC) USING BTREE,
  INDEX `idx_status`(`status` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '科室表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of department
-- ----------------------------
INSERT INTO `department` VALUES (1782362925000001, '呼吸内科', '3层', '门诊楼3层东侧A区', '诊疗呼吸系统疾病', '010-12345601', 1, 1, 0, '2026-06-16 15:55:52', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000002, '心内科', '4层', '门诊楼4层西侧B区', '诊疗心血管系统疾病', '010-12345602', 2, 1, 0, '2026-06-16 15:55:52', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000003, '神经内科', '5层', '门诊楼5层南侧C区', '诊疗神经系统疾病', '010-12345603', 3, 1, 0, '2026-06-16 15:55:52', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000004, '消化内科', '3层', '门诊楼3层西侧B区', '诊治食管、胃、肠、肝、胆、胰等消化系统疾病，开展胃镜、肠镜检查', '010-12345602', 2, 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000005, '皮肤科', '2层', '门诊楼2层东侧C区', '诊治各类皮肤病、性病、皮肤美容，开展激光治疗、冷冻治疗', '010-12345603', 3, 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000006, '儿科', '2层', '门诊楼2层西侧儿童诊区', '14岁以下儿童内科疾病诊治，设有儿童候诊游乐区', '010-12345604', 4, 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000007, '妇产科', '6层', '门诊楼6层', '产科检查、妇科疾病诊治、产前检查、产后康复、计划生育', '010-12345605', 5, 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000008, '眼科', '7层', '门诊楼7层东侧', '白内障、青光眼、眼底病、屈光不正等眼科疾病诊治与手术', '010-12345606', 6, 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000009, '内分泌科', '4层', '门诊楼4层东侧A区', '糖尿病、甲状腺疾病、骨质疏松、肥胖症等内分泌代谢疾病诊治', '010-12345612', 8, 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000010, '急诊科', '1层', '门诊楼1层西侧（急诊通道）', '24小时急诊抢救、留观，处理各类急危重症', '010-12345620', 99, 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `department` VALUES (1782362925000011, '骨科', '5层', '门诊楼5层北侧D区', '骨科诊疗范围：骨折、关节脱位、颈椎病、腰椎间盘突出、骨关节炎、运动损伤等。开展关节镜微创手术、人工关节置换。', '0111-225263', 1, 1, 0, '2026-06-17 11:11:51', '2026-06-25 12:48:45');

SET FOREIGN_KEY_CHECKS = 1;
