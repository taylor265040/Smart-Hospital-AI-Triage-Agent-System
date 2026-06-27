/**
 * 统一 API 请求模块 — JWT 认证拦截器
 * 自动附加 Authorization header，401 时自动刷新令牌
 */

const BASE = '/api'

// ── 令牌管理（使用 sessionStorage 按标签隔离会话，避免多标签共享 localStorage 串扰） ──

function getAuth() {
  try {
    const saved = sessionStorage.getItem('auth')
    return saved ? JSON.parse(saved) : null
  } catch { return null }
}

function setAuth(auth) {
  sessionStorage.setItem('auth', JSON.stringify(auth))
}

function clearAuth() {
  sessionStorage.removeItem('auth')
  sessionStorage.removeItem('user')
}

// ── 刷新令牌 ──

let refreshPromise = null // 防并发：多个 401 同时触发时只调一次刷新

async function tryRefreshToken() {
  if (refreshPromise) return refreshPromise

  const auth = getAuth()
  if (!auth?.refreshToken) return false

  refreshPromise = (async () => {
    try {
      const res = await fetch(`${BASE}/auth/refresh`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ refreshToken: auth.refreshToken })
      })
      const r = await res.json()
      if (r.code === 200 && r.data) {
        setAuth({
          accessToken: r.data.accessToken,
          refreshToken: r.data.refreshToken,
          expiresAt: r.data.expiresAt
        })
        return true
      }
      return false
    } catch {
      return false
    } finally {
      refreshPromise = null
    }
  })()

  return refreshPromise
}

// ── 核心请求封装 ──

async function request(url, options = {}) {
  const auth = getAuth()
  const headers = { 'Content-Type': 'application/json', ...(options.headers || {}) }

  // 自动附加 JWT
  if (auth?.accessToken) {
    headers['Authorization'] = `Bearer ${auth.accessToken}`
  }

  let res = await fetch(url, { ...options, headers })

  // 401 → 尝试刷新令牌后重试一次
  if (res.status === 401 && auth?.refreshToken) {
    const refreshed = await tryRefreshToken()
    if (refreshed) {
      const newAuth = getAuth()
      headers['Authorization'] = `Bearer ${newAuth.accessToken}`
      res = await fetch(url, { ...options, headers })
    }
  }

  const r = await res.json()

  // 刷新后仍 401 → 强制退出
  if (r.code === 401) {
    clearAuth()
    window.location.replace('/login')
    throw new Error('登录已过期，请重新登录')
  }

  if (r.code !== 200) throw new Error(r.message || '请求失败')
  return r.data
}

// ── 公开导出 ──

/** 暴露 auth 操作供 store 使用 */
export const authStorage = { getAuth, setAuth, clearAuth }

/** 通用请求 */
export { request }

// ── 各模块 API ──

export const userApi = {
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/user/list?${qs}`) },
  detail(id) { return request(`${BASE}/user/${id}`) },
  create(body) { return request(`${BASE}/user`, { method: 'POST', body: JSON.stringify(body) }) },
  update(body) { return request(`${BASE}/user`, { method: 'PUT', body: JSON.stringify(body) }) },
  delete(id) { return request(`${BASE}/user/${id}`, { method: 'DELETE' }) },
  batchDelete(ids) { return request(`${BASE}/user/batch`, { method: 'DELETE', body: JSON.stringify(ids) }) }
}

export const deptApi = {
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/department/list?${qs}`) },
  all() { return request(`${BASE}/department/all`) },
  detail(id) { return request(`${BASE}/department/${id}`) },
  create(body) { return request(`${BASE}/department`, { method: 'POST', body: JSON.stringify(body) }) },
  update(body) { return request(`${BASE}/department`, { method: 'PUT', body: JSON.stringify(body) }) },
  delete(id) { return request(`${BASE}/department/${id}`, { method: 'DELETE' }) },
  batchDelete(ids) { return request(`${BASE}/department/batch`, { method: 'DELETE', body: JSON.stringify(ids) }) }
}

export const doctorApi = {
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/doctor/list?${qs}`) },
  detail(id) { return request(`${BASE}/doctor/${id}`) },
  create(body) { return request(`${BASE}/doctor`, { method: 'POST', body: JSON.stringify(body) }) },
  update(body) { return request(`${BASE}/doctor`, { method: 'PUT', body: JSON.stringify(body) }) },
  delete(id) { return request(`${BASE}/doctor/${id}`, { method: 'DELETE' }) },
  batchDelete(ids) { return request(`${BASE}/doctor/batch`, { method: 'DELETE', body: JSON.stringify(ids) }) }
}

export const kbApi = {
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/knowledge-base/list?${qs}`) },
  detail(id) { return request(`${BASE}/knowledge-base/${id}`) },
  create(body) { return request(`${BASE}/knowledge-base`, { method: 'POST', body: JSON.stringify(body) }) },
  update(body) { return request(`${BASE}/knowledge-base`, { method: 'PUT', body: JSON.stringify(body) }) },
  delete(id) { return request(`${BASE}/knowledge-base/${id}`, { method: 'DELETE' }) },
  batchDelete(ids) { return request(`${BASE}/knowledge-base/batch`, { method: 'DELETE', body: JSON.stringify(ids) }) }
}

export const convApi = {
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/conversation/list?${qs}`) },
  detail(id) { return request(`${BASE}/conversation/${id}`) },
  create(body) { return request(`${BASE}/conversation`, { method: 'POST', body: JSON.stringify(body) }) },
  update(body) { return request(`${BASE}/conversation`, { method: 'PUT', body: JSON.stringify(body) }) },
  delete(id) { return request(`${BASE}/conversation/${id}`, { method: 'DELETE' }) },
  batchDelete(ids) { return request(`${BASE}/conversation/batch`, { method: 'DELETE', body: JSON.stringify(ids) }) }
}

export const msgApi = {
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/message/list?${qs}`) },
  byConversation(convId) { return request(`${BASE}/message/conversation/${convId}`) },
  detail(id) { return request(`${BASE}/message/${id}`) },
  create(body) { return request(`${BASE}/message`, { method: 'POST', body: JSON.stringify(body) }) },
  delete(id) { return request(`${BASE}/message/${id}`, { method: 'DELETE' }) },
  batchDelete(ids) { return request(`${BASE}/message/batch`, { method: 'DELETE', body: JSON.stringify(ids) }) }
}

export const annApi = {
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/announcement/list?${qs}`) },
  published() { return request(`${BASE}/announcement/published`) },
  detail(id) { return request(`${BASE}/announcement/${id}`) },
  create(body) { return request(`${BASE}/announcement`, { method: 'POST', body: JSON.stringify(body) }) },
  update(body) { return request(`${BASE}/announcement`, { method: 'PUT', body: JSON.stringify(body) }) },
  publish(id) { return request(`${BASE}/announcement/publish/${id}`, { method: 'PUT' }) },
  delete(id) { return request(`${BASE}/announcement/${id}`, { method: 'DELETE' }) },
  batchDelete(ids) { return request(`${BASE}/announcement/batch`, { method: 'DELETE', body: JSON.stringify(ids) }) }
}

export const ratingApi = {
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/rating/list?${qs}`) },
  detail(id) { return request(`${BASE}/rating/${id}`) },
  create(body) { return request(`${BASE}/rating`, { method: 'POST', body: JSON.stringify(body) }) },
  delete(id) { return request(`${BASE}/rating/${id}`, { method: 'DELETE' }) },
  batchDelete(ids) { return request(`${BASE}/rating/batch`, { method: 'DELETE', body: JSON.stringify(ids) }) }
}

export const apptApi = {
  departments() { return request(`${BASE}/appointment/departments`) },
  doctors(deptId) { return request(`${BASE}/appointment/doctors?departmentId=${deptId}`) },
  slots(doctorId, date) { return request(`${BASE}/appointment/slots?doctorId=${doctorId}&date=${date}`) },
  book(body) { return request(`${BASE}/appointment/book`, { method: 'POST', body: JSON.stringify(body) }) },
  list(params = {}) { const qs = new URLSearchParams(params).toString(); return request(`${BASE}/appointment/list?${qs}`) },
  detail(id) { return request(`${BASE}/appointment/${id}`) },
  cancel(id, userId) { return request(`${BASE}/appointment/${id}/cancel?userId=${userId}`, { method: 'PUT' }) },
  confirm(id, doctorId) { return request(`${BASE}/appointment/${id}/confirm?doctorId=${doctorId}`, { method: 'PUT' }) }
}

export const qaApi = {
  ask(question, userId, conversationId) {
    return request(`${BASE}/v2/qa/ask`, { method: 'POST', body: JSON.stringify({ question, userId, conversationId }) })
  },
  refreshIndex() { return request(`${BASE}/v2/qa/refresh-index`, { method: 'POST' }) },
  health() { return request(`${BASE}/v2/qa/health`) }
}

export const csApi = {
  requestHuman(userId, reason = '') {
    return request(`${BASE}/cs/request-human?userId=${userId}${reason ? '&reason=' + reason : ''}`, { method: 'POST' })
  },
  queueStatus(userId) { return request(`${BASE}/cs/queue-status?userId=${userId}`) },
  cancelQueue(userId) { return request(`${BASE}/cs/cancel-queue?userId=${userId}`, { method: 'POST' }) },
  staffOnline(csStaffId) { return request(`${BASE}/cs/staff-online?csStaffId=${csStaffId}`, { method: 'POST' }) },
  staffOffline(csStaffId) { return request(`${BASE}/cs/staff-offline?csStaffId=${csStaffId}`, { method: 'POST' }) },
  pendingQueue() { return request(`${BASE}/cs/pending-queue`) },
  accept(csStaffId, convId) { return request(`${BASE}/cs/accept?csStaffId=${csStaffId}&conversationId=${convId}`, { method: 'POST' }) },
  closeSession(csStaffId, convId) { return request(`${BASE}/cs/close-session?csStaffId=${csStaffId}&conversationId=${convId}`, { method: 'POST' }) },
  onlineStaff() { return request(`${BASE}/cs/online-staff`) }
}

/** 登录接口（不经过 JWT 拦截，request 会自动处理） */
export function loginApi(username, password) {
  return fetch(`${BASE}/auth/login`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ username, password })
  }).then(r => r.json())
}

/** 人工关键词检测 */
const HUMAN_KEYWORDS = ['人工', '转人工', '找客服', '投诉', '人工客服', '真人', '转接']
export function shouldTriggerHuman(text) {
  return HUMAN_KEYWORDS.some(kw => text.includes(kw))
}
