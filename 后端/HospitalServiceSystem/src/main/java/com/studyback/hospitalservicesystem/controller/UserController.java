package com.studyback.hospitalservicesystem.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.studyback.hospitalservicesystem.common.PageResult;
import com.studyback.hospitalservicesystem.common.Result;
import com.studyback.hospitalservicesystem.entity.User;
import com.studyback.hospitalservicesystem.service.UserService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.RequiredArgsConstructor;
import org.springframework.util.DigestUtils;
import org.springframework.util.StringUtils;
import org.springframework.web.bind.annotation.*;

import java.nio.charset.StandardCharsets;
import java.util.List;

/**
 * 用户控制器
 */
@Tag(name = "用户管理", description = "用户的CRUD操作")
@RestController
@RequestMapping("/api/user")
@RequiredArgsConstructor
public class UserController {

    private final UserService userService;

    @Operation(summary = "分页查询用户列表")
    @GetMapping("/list")
    public Result<PageResult<User>> list(
            @Parameter(description = "页码") @RequestParam(defaultValue = "1") Integer pageNum,
            @Parameter(description = "每页大小") @RequestParam(defaultValue = "10") Integer pageSize,
            @Parameter(description = "用户名/姓名（模糊搜索）") @RequestParam(required = false) String keyword,
            @Parameter(description = "角色") @RequestParam(required = false) String role) {

        Page<User> page = userService.pageWithCondition(pageNum, pageSize, keyword, role);
        return Result.ok(PageResult.of(page));
    }

    @Operation(summary = "查询用户详情")
    @GetMapping("/{id}")
    public Result<User> getById(@Parameter(description = "用户ID") @PathVariable Long id) {
        User user = userService.getByIdOrNull(id);
        if (user == null) {
            return Result.notFound("用户不存在");
        }
        return Result.ok(user);
    }

    @Operation(summary = "新增用户")
    @PostMapping
    public Result<User> create(@RequestBody User user) {
        if (userService.existsByUsername(user.getUsername(), null)) {
            return Result.fail("用户名已存在");
        }
        // 密码 MD5 加密存储
        if (StringUtils.hasText(user.getPassword())) {
            user.setPassword(DigestUtils.md5DigestAsHex(user.getPassword().getBytes(StandardCharsets.UTF_8)));
        }
        userService.save(user);
        // 返回前清除密码字段
        user.setPassword(null);
        return Result.ok("新增成功", user);
    }

    @Operation(summary = "更新用户")
    @PutMapping
    public Result<User> update(@RequestBody User user) {
        if (user.getId() == null) {
            return Result.badRequest("用户ID不能为空");
        }
        User exist = userService.getByIdOrNull(user.getId());
        if (exist == null) {
            return Result.notFound("用户不存在");
        }
        if (userService.existsByUsername(user.getUsername(), user.getId())) {
            return Result.fail("用户名已存在");
        }
        // 密码处理：传了新密码则 MD5 加密，没传则保留原密码
        if (StringUtils.hasText(user.getPassword())) {
            user.setPassword(DigestUtils.md5DigestAsHex(user.getPassword().getBytes(StandardCharsets.UTF_8)));
        } else {
            user.setPassword(exist.getPassword());
        }
        userService.updateById(user);
        // 返回前清除密码
        User result = userService.getByIdOrNull(user.getId());
        if (result != null) result.setPassword(null);
        return Result.ok("更新成功", result);
    }

    @Operation(summary = "删除用户")
    @DeleteMapping("/{id}")
    public Result<Void> delete(@Parameter(description = "用户ID") @PathVariable Long id) {
        boolean removed = userService.removeById(id);
        if (!removed) {
            return Result.notFound("用户不存在");
        }
        return Result.ok();
    }

    @Operation(summary = "批量删除用户")
    @DeleteMapping("/batch")
    public Result<Void> deleteBatch(@Parameter(description = "用户ID列表") @RequestBody List<Long> ids) {
        userService.removeByIds(ids);
        return Result.ok();
    }

}
