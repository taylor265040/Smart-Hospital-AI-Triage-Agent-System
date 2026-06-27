<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../stores/user'
import { Stethoscope, User, MessageSquare, Settings, Sparkles, AlertCircle } from 'lucide-vue-next'

const router = useRouter()
const store = useUserStore()

const roles = [
  { value: 'PATIENT', label: '患者', icon: User, user: 'zhangsan', pass: '123456', desc: '智能问诊 / 预约挂号 / 人工咨询' },
  { value: 'CS_STAFF', label: '客服', icon: MessageSquare, user: 'cs001', pass: '123456', desc: '排队管理 / 一对一即时通讯' },
  { value: 'DOCTOR', label: '医生', icon: Stethoscope, user: 'doctor1', pass: '123456', desc: '在岗管理 / 约诊流水查询' },
  { value: 'ADMIN', label: '管理员', icon: Settings, user: 'admin', pass: '123456', desc: '医院主数据 CRUD / 批量管控' }
]

const selectedRole = ref('PATIENT')
const username = ref('zhangsan')
const password = ref('123456')
const loading = ref(false)
const error = ref('')

function selectRole(r) {
  selectedRole.value = r.value
  username.value = r.user
  password.value = r.pass
  error.value = ''
}

async function handleLogin() {
  error.value = ''
  if (!username.value.trim() || !password.value.trim()) {
    error.value = '请输入用户名和密码'
    return
  }
  loading.value = true
  try {
    const u = await store.login(username.value.trim(), password.value.trim())
    const redirect = { PATIENT: '/patient', CS_STAFF: '/cs', DOCTOR: '/doctor', ADMIN: '/admin' }
    router.push(redirect[u.role] || '/patient')
  } catch (e) {
    error.value = e.message || '登录失败'
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <div class="min-h-screen bg-slate-50 flex items-center justify-center p-4">
    <div class="bg-white rounded-3xl shadow-2xl overflow-hidden max-w-6xl w-full grid grid-cols-1 md:grid-cols-5">
      <!-- Left Brand Panel -->
      <div class="md:col-span-2 bg-gradient-to-br from-slate-900 via-blue-950 to-blue-900 text-white p-8 md:p-10 flex flex-col justify-between">
        <div>
          <div class="bg-blue-600 text-white p-3 rounded-2xl inline-block shadow-lg mb-6">
            <Stethoscope class="w-8 h-8" />
          </div>
          <h1 class="text-3xl md:text-4xl font-bold tracking-tight mb-3">医院智慧客服系统</h1>
          <p class="text-blue-200 text-base mb-8">基于 AI Agent + 多知识库 RAG 的智慧医疗平台</p>
          <div class="space-y-3">
            <div v-for="r in roles" :key="r.value" class="flex items-center gap-2.5 text-base text-blue-100/70">
              <component :is="r.icon" class="w-5 h-5 text-blue-400 shrink-0" />
              <span>{{ r.desc }}</span>
            </div>
          </div>
        </div>
        <div class="text-xs text-blue-300/50 mt-8">
          <Sparkles class="w-3 h-3 inline mr-1" /> v2.0 · 检索增强生成
        </div>
      </div>

      <!-- Right Login Form -->
      <div class="md:col-span-3 p-8 md:p-10">
        <h2 class="text-2xl font-bold text-slate-900 mb-2">登录系统</h2>
        <p class="text-base text-slate-500 mb-8">选择角色后输入账号密码</p>

        <!-- Role Tabs -->
        <div class="bg-slate-100 p-1.5 rounded-xl grid grid-cols-4 gap-1 mb-6">
          <button v-for="r in roles" :key="r.value" @click="selectRole(r)"
            :class="['px-3 py-2.5 rounded-xl text-sm font-semibold flex flex-col items-center gap-1 transition-all',
              selectedRole === r.value ? 'bg-white shadow-sm text-blue-700' : 'text-slate-500 hover:text-slate-700 hover:bg-white/50']">
            <component :is="r.icon" class="w-4 h-4" />
            <span>{{ r.label }}</span>
          </button>
        </div>

        <!-- Form -->
        <div class="space-y-4">
          <div>
            <label class="block text-sm font-medium text-slate-500 mb-2">用户名</label>
            <input v-model="username" @keyup.enter="handleLogin"
              class="w-full bg-slate-50 border border-slate-200 focus:bg-white focus:border-blue-500 rounded-xl px-4 py-3 text-base outline-none transition-all placeholder:text-slate-400"
              placeholder="请输入用户名" />
          </div>
          <div>
            <label class="block text-sm font-medium text-slate-500 mb-2">密码</label>
            <input v-model="password" type="password" @keyup.enter="handleLogin"
              class="w-full bg-slate-50 border border-slate-200 focus:bg-white focus:border-blue-500 rounded-xl px-4 py-3 text-base outline-none transition-all placeholder:text-slate-400"
              placeholder="请输入密码" />
          </div>

          <div v-if="error" class="bg-rose-50 border border-rose-200 text-rose-700 rounded-xl p-3 text-xs flex items-center gap-2">
            <AlertCircle class="w-4 h-4 text-rose-500 shrink-0" />
            {{ error }}
          </div>

          <button @click="handleLogin" :disabled="loading"
            class="w-full bg-blue-600 hover:bg-blue-700 active:bg-blue-800 disabled:bg-blue-400 text-white font-bold py-3 rounded-xl text-base transition-all shadow-md flex items-center justify-center gap-2">
            <span v-if="loading" class="inline-block w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin"></span>
            {{ loading ? '验证中...' : '登录系统' }}
          </button>
        </div>
      </div>
    </div>
  </div>
</template>
