package com.studyback.hospitalservicesystem.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.time.LocalDateTime;

/**
 * 科室实体
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@TableName("department")
public class Department implements Serializable {

    private static final long serialVersionUID = 1L;

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /** 科室名称 */
    private String name;

    /** 所在楼层 */
    private String floor;

    /** 具体位置 */
    private String location;

    /** 科室描述 */
    private String description;

    /** 科室电话 */
    private String phone;

    /** 排序 */
    private Integer sortOrder;

    /** 状态：1-启用, 0-禁用 */
    private Integer status;

    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updateTime;

}
