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

 Date: 25/06/2026 12:51:48
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for doctor
-- ----------------------------
DROP TABLE IF EXISTS `doctor`;
CREATE TABLE `doctor`  (
  `id` bigint(20) NOT NULL COMMENT '主键ID',
  `name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '医生姓名',
  `title` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '职称',
  `department_id` bigint(20) NULL DEFAULT NULL COMMENT '所属科室ID',
  `specialization` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '专长',
  `description` text CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL COMMENT '医生简介',
  `schedule` text CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL COMMENT '排班信息JSON',
  `consultation_location` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '问诊地点（如：3层呼吸内科A区3诊室）',
  `status` int(11) NOT NULL DEFAULT 1 COMMENT '状态',
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT '逻辑删除',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `idx_department_id`(`department_id` ASC) USING BTREE,
  INDEX `idx_name`(`name` ASC) USING BTREE,
  INDEX `idx_status`(`status` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '医生表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of doctor
-- ----------------------------
INSERT INTO `doctor` VALUES (1, '张伟', '主任医师', 1782362925000002, '冠心病、高血压、心律失常', '心内科主任，从事心血管临床工作20余年', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼4层西侧B区 心内科2诊室', 1, 0, '2026-06-16 15:55:52', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (2, '王芳', '副主任医师', 1782362925000001, '哮喘、慢阻肺、肺部感染', '呼吸内科副主任，擅长呼吸系统疑难杂症诊治', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼3层东侧A区 呼吸内科1诊室', 1, 0, '2026-06-16 15:55:52', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (3, '李明', '主治医师', 1782362925000003, '头痛、眩晕、脑血管疾病', '神经内科主治医师，专注脑血管疾病介入治疗', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼5层南侧C区 神经内科3诊室', 1, 0, '2026-06-16 15:55:52', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (4, '赵敏', '主任医师', 1782362925000011, '骨折、关节置换、运动损伤', '骨科主任，擅长微创关节镜手术', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', NULL, 1, 0, '2026-06-16 15:55:52', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (10, '陈强', '主任医师', 1782362925000001, '慢性阻塞性肺疾病、支气管哮喘、肺部感染', '从事呼吸内科临床工作30年，中华医学会呼吸病学分会委员', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼3层东侧A区1诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (11, '刘洋', '副主任医师', 1782362925000002, '冠心病介入治疗、高血压、心律失常', '擅长冠脉支架植入术，年手术量300余例', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼4层西侧B区3诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (12, '周文', '主任医师', 1782362925000003, '脑血管疾病、帕金森病、癫痫', '神经内科学科带头人，从事临床工作25年', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼5层南侧C区1诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (13, '孙丽', '副主任医师', 1782362925000004, '胃肠疾病、肝胆疾病、内镜诊治', '擅长无痛胃镜、结肠镜检查及内镜下治疗', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼3层西侧B区2诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (14, '黄蓉', '主治医师', 1782362925000005, '湿疹、银屑病、痤疮、皮肤激光美容', '皮肤科青年骨干，擅长激光祛斑、点阵激光', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼2层东侧C区皮肤科诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (15, '吴敏', '主任医师', 1782362925000006, '儿童呼吸系统疾病、新生儿疾病、儿童保健', '儿科主任，从事儿科临床工作28年，深受患儿家长信赖', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼2层西侧儿童诊区1诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (16, '郑华', '主任医师', 1782362925000007, '高危妊娠管理、妇科肿瘤、微创手术', '妇产科主任，擅长腹腔镜子宫肌瘤剔除、卵巢囊肿切除', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼6层妇产科1诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (17, '钱峰', '副主任医师', 1782362925000008, '白内障超声乳化手术、青光眼、眼底病', '完成白内障手术逾5000例', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼7层东侧眼科诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (18, '林洁', '主治医师', 1782362925000009, '糖尿病综合管理、甲状腺结节、痛风', '擅长糖尿病饮食运动指导及胰岛素泵治疗', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼4层东侧A区2诊室', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');
INSERT INTO `doctor` VALUES (19, '杨刚', '副主任医师', 1782362925000010, '急危重症抢救、中毒、多发伤', '急诊科副主任，从事急诊工作15年', '{\"weekdays\":[1,2,3,4,5],\"morning\":\"08:00-12:00\",\"afternoon\":\"14:00-18:00\"}', '门诊楼1层急诊科', 1, 0, '2026-06-25 12:20:26', '2026-06-25 12:48:45');

SET FOREIGN_KEY_CHECKS = 1;
