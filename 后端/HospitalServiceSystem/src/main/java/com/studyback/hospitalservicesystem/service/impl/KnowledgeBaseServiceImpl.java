package com.studyback.hospitalservicesystem.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.studyback.hospitalservicesystem.entity.KnowledgeBase;
import com.studyback.hospitalservicesystem.mapper.KnowledgeBaseMapper;
import com.studyback.hospitalservicesystem.service.KnowledgeBaseService;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import java.util.List;

/**
 * 知识库服务实现
 */
@Service
public class KnowledgeBaseServiceImpl extends ServiceImpl<KnowledgeBaseMapper, KnowledgeBase> implements KnowledgeBaseService {

    @Override
    public Page<KnowledgeBase> pageWithCondition(Integer pageNum, Integer pageSize, String keyword, String category, Integer status) {
        Page<KnowledgeBase> page = new Page<>(pageNum, pageSize);
        LambdaQueryWrapper<KnowledgeBase> wrapper = new LambdaQueryWrapper<>();

        if (StringUtils.hasText(keyword)) {
            wrapper.and(w -> w.like(KnowledgeBase::getQuestion, keyword)
                    .or().like(KnowledgeBase::getAnswer, keyword));
        }
        if (StringUtils.hasText(category)) {
            wrapper.eq(KnowledgeBase::getCategory, category);
        }
        if (status != null) {
            wrapper.eq(KnowledgeBase::getStatus, status);
        }
        wrapper.orderByDesc(KnowledgeBase::getViewCount)
                .orderByDesc(KnowledgeBase::getCreateTime);

        return this.page(page, wrapper);
    }

    @Override
    public KnowledgeBase getByIdAndIncrementView(Long id) {
        KnowledgeBase kb = this.getById(id);
        if (kb != null) {
            kb.setViewCount((kb.getViewCount() == null ? 0 : kb.getViewCount()) + 1);
            this.updateById(kb);
        }
        return kb;
    }

    @Override
    public KnowledgeBase getByIdOrNull(Long id) {
        return this.getById(id);
    }

    @Override
    public List<KnowledgeBase> listPublishedByKbType(String kbType) {
        return this.list(new LambdaQueryWrapper<KnowledgeBase>()
                .eq(KnowledgeBase::getStatus, 1)
                .eq(StringUtils.hasText(kbType), KnowledgeBase::getKbType, kbType));
    }

    @Override
    public List<KnowledgeBase> listAllPublished() {
        return this.list(new LambdaQueryWrapper<KnowledgeBase>()
                .eq(KnowledgeBase::getStatus, 1));
    }

}
