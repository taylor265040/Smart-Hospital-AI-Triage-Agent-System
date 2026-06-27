import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { loginApi, authStorage } from '../api'

export const useUserStore = defineStore('user', () => {
  const user = ref(null)

  const isLoggedIn = computed(() => !!user.value && !!authStorage.getAuth()?.accessToken)
  const role = computed(() => user.value?.role)

  /** 检查 token 是否过期 */
  function isTokenExpired() {
    const auth = authStorage.getAuth()
    if (!auth?.expiresAt) return true
    // 提前 60 秒视为过期，避免边缘情况
    return Date.now() > auth.expiresAt - 60000
  }

  /** 登录 */
  async function login(username, password) {
    const res = await loginApi(username, password)
    if (res.code !== 200) throw new Error(res.message || '登录失败')

    const { accessToken, refreshToken, expiresAt, userId, realName, role: userRole } = res.data

    // 持久化令牌
    authStorage.setAuth({ accessToken, refreshToken, expiresAt })

    // 持久化用户信息（供路由守卫快速读取角色）—— sessionStorage 按标签隔离
    const userData = { id: userId, username, realName, role: userRole }
    user.value = userData
    sessionStorage.setItem('user', JSON.stringify(userData))

    return userData
  }

  /** 退出登录 */
  function logout() {
    user.value = null
    authStorage.clearAuth()
  }

  /** 从 sessionStorage 恢复会话（用于页面刷新，按标签隔离） */
  function restoreSession() {
    const saved = sessionStorage.getItem('user')
    const auth = authStorage.getAuth()
    if (saved && auth?.accessToken && !isTokenExpired()) {
      try { user.value = JSON.parse(saved) } catch {}
      return true
    }
    // token 过期或不存在 → 清理
    logout()
    return false
  }

  return { user, isLoggedIn, role, isTokenExpired, login, logout, restoreSession }
})
