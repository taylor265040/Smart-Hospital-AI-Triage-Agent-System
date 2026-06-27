package com.studyback.hospitalservicesystem.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.time.LocalDateTime;

/**
 * 知识库实体（FAQ）
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@TableName("knowledge_base")
public class KnowledgeBase implements Serializable {

    private static final long serialVersionUID = 1L;

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /** 问题 */
    private String question;

    /** 答案 */
    private String answer;

    /** 分类 */
    private String category;

    /** 知识库类型：FAQ / DOCTOR / DEPARTMENT / INSURANCE / NAVIGATION */
    private String kbType;

    /** 关联科室ID */
    private Long departmentId;

    /** 浏览次数 */
    private Integer viewCount;

    /** 状态：1-已发布, 0-草稿, 2-待审核 */
    private Integer status;

    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updateTime;

}
