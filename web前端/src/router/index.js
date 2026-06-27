import { createRouter, createWebHistory } from 'vue-router'
import { authStorage } from '../api'

const routes = [
  {
    path: '/login',
    name: 'Login',
    component: () => import('../views/LoginView.vue'),
    meta: { title: '登录 — 医院智慧客服系统' }
  },
  {
    path: '/patient',
    name: 'Patient',
    component: () => import('../views/patient/PatientView.vue'),
    meta: { role: 'PATIENT', title: '患者端 — 智能问诊' }
  },
  {
    path: '/cs',
    name: 'CS',
    component: () => import('../views/cs/CSView.vue'),
    meta: { role: 'CS_STAFF', title: '客服端 — 工作台' }
  },
  {
    path: '/doctor',
    name: 'Doctor',
    component: () => import('../views/doctor/DoctorView.vue'),
    meta: { role: 'DOCTOR', title: '医生端 — 约诊流水' }
  },
  {
    path: '/admin',
    name: 'Admin',
    component: () => import('../views/admin/AdminView.vue'),
    meta: { role: 'ADMIN', title: '管理端 — 主数据管理' }
  },
  { path: '/', redirect: '/login' },
  { path: '/:pathMatch(.*)*', redirect: '/login' }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

const roleRedirect = { PATIENT: '/patient', CS_STAFF: '/cs', DOCTOR: '/doctor', ADMIN: '/admin' }

router.beforeEach((to, from, next) => {
  // 读取 sessionStorage 中的用户和令牌（按标签隔离）
  const saved = sessionStorage.getItem('user')
  let user = null
  try { user = saved ? JSON.parse(saved) : null } catch { sessionStorage.removeItem('user') }

  // 校验用户数据完整性
  if (user && (!user.id || !user.role || !roleRedirect[user.role])) {
    sessionStorage.removeItem('user')
    authStorage.clearAuth()
    user = null
  }

  // 校验 JWT 是否有效
  const auth = authStorage.getAuth()
  if (user && !auth?.accessToken) {
    // 有用户数据但没有 token → 清理
    sessionStorage.removeItem('user')
    user = null
  }

  if (to.path === '/login') {
    if (user) return next(roleRedirect[user.role] || '/patient')
    return next()
  }

  if (!user) return next('/login')

  const requiredRole = to.meta?.role
  if (requiredRole && user.role !== requiredRole) {
    return next(roleRedirect[user.role] || '/patient')
  }

  document.title = to.meta.title || '医院智慧客服系统'
  next()
})

export default router
