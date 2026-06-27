package com.studyback.hospitalservicesystem.service;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.studyback.hospitalservicesystem.entity.KnowledgeBase;

import java.util.List;

/**
 * 知识库服务接口
 */
public interface KnowledgeBaseService extends IService<KnowledgeBase> {

    /**
     * 分页条件查询知识库
     */
    Page<KnowledgeBase> pageWithCondition(Integer pageNum, Integer pageSize, String keyword, String category, Integer status);

    /**
     * 根据ID查询并增加浏览次数
     */
    KnowledgeBase getByIdAndIncrementView(Long id);

    /**
     * 根据ID查询，不存在返回null
     */
    KnowledgeBase getByIdOrNull(Long id);

    /**
     * 按知识库类型查询已发布条目（供 Agent RAG 索引使用）
     */
    List<KnowledgeBase> listPublishedByKbType(String kbType);

    /**
     * 查询所有已发布条目
     */
    List<KnowledgeBase> listAllPublished();

}
