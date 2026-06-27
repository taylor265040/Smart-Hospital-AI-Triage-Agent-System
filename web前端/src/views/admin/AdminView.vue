<script setup>
import { ref, onMounted, computed } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../../stores/user'
import {
  Settings, Users, Layers, BookOpen, Star, Plus, RefreshCw,
  CheckCircle, X, AlertCircle, Search, Trash2, Edit3, Eye,
  Building2, Stethoscope, Bell, MessageSquare, Shield
} from 'lucide-vue-next'
import { request as api } from '../../api'

const store = useUserStore()
const router = useRouter()
const user = store.user

// ── Tabs ──
const tabs = [
  { key: 'users', label: '用户', icon: Users, color: 'blue' },
  { key: 'depts', label: '科室', icon: Building2, color: 'emerald' },
  { key: 'docs', label: '医生', icon: Stethoscope, color: 'indigo' },
  { key: 'kb', label: '知识库', icon: BookOpen, color: 'purple' },
  { key: 'announcements', label: '公告', icon: Bell, color: 'amber' },
  { key: 'ratings', label: '评价', icon: Star, color: 'rose' },
]
const activeTab = ref('users')
const selectedIds = ref([])
const searchQuery = ref('')

// ── Data ──
const adminUsers = ref([]); const adminDepts = ref([]); const adminDocs = ref([])
const adminKb = ref([]); const adminAnnouncements = ref([]); const adminRatings = ref([])

// ── Modal State ──
const showFormModal = ref(false)
const showDeleteModal = ref(false)
const deleteTarget = ref(null)     // { id, label } — single delete
const isBulkDelete = ref(false)
const editingType = ref('user')
const editingPayload = ref({})
const formErrors = ref({})

// ── Toast ──
const actionMessage = ref(null)
const isGlobalLoading = ref(false)

function showToast(text, type = 'success') {
  actionMessage.value = { text, type }
  setTimeout(() => { if (actionMessage.value?.text === text) actionMessage.value = null }, 4000)
}

// ── Data Fetching ──
async function fetchAll() {
  isGlobalLoading.value = true
  try {
    const ps = 500
    const [u, d, docs, k, a, r] = await Promise.all([
      api(`/api/user/list?pageSize=${ps}`),
      api(`/api/department/list?pageSize=${ps}`),
      api(`/api/doctor/list?pageSize=${ps}`),
      api(`/api/knowledge-base/list?pageSize=${ps}`),
      api(`/api/announcement/list?pageSize=${ps}`),
      api(`/api/rating/list?pageSize=${ps}`),
    ])
    adminUsers.value = u.records || []
    adminDepts.value = (d.records || []).sort((a, b) => {
      const na = parseInt(a.floor) || 99, nb = parseInt(b.floor) || 99
      return na - nb || (a.sortOrder || 0) - (b.sortOrder || 0)
    })
    adminDocs.value = docs.records || []
    adminKb.value = k.records || []
    adminAnnouncements.value = a.records || []
    adminRatings.value = r.records || []
    selectedIds.value = []
  } catch (e) { console.error(e) }
  finally { isGlobalLoading.value = false }
}

// ── Computed ──
function getList() {
  const map = { users: adminUsers, depts: adminDepts, docs: adminDocs, kb: adminKb, announcements: adminAnnouncements, ratings: adminRatings }
  let list = map[activeTab.value]?.value || []
  if (searchQuery.value.trim()) {
    const q = searchQuery.value.trim().toLowerCase()
    list = list.filter(item => JSON.stringify(item).toLowerCase().includes(q))
  }
  return list
}

const toggleAll = computed({
  get: () => { const l = getList(); return l.length > 0 && selectedIds.value.length === l.length },
  set: v => { selectedIds.value = v ? getList().map(i => i.id) : [] }
})

const statsCards = computed(() => [
  { label: '用户总数', value: adminUsers.value.length, unit: '位', icon: Users, color: 'blue' },
  { label: '科室数量', value: adminDepts.value.length, unit: '个', icon: Building2, color: 'emerald' },
  { label: '医生数量', value: adminDocs.value.length, unit: '位', icon: Stethoscope, color: 'indigo' },
  { label: '知识库', value: adminKb.value.length, unit: '条', icon: BookOpen, color: 'purple' },
  { label: '公告', value: adminAnnouncements.value.length, unit: '条', icon: Bell, color: 'amber' },
  { label: '评价', value: adminRatings.value.length, unit: '笔', icon: Star, color: 'rose' },
])

// ── Helpers ──
function getDeptName(id) { return adminDepts.value.find(d => d.id === id)?.name || `#${id}` }
function getUserName(id) { return adminUsers.value.find(u => u.id === id)?.realName || `#${id}` }
function getRoleLabel(r) { return { ADMIN: '管理员', DOCTOR: '医生', CS_STAFF: '客服', PATIENT: '患者' }[r] || r }
function getRoleColor(r) { return { ADMIN: 'bg-rose-50 text-rose-700', DOCTOR: 'bg-blue-50 text-blue-700', CS_STAFF: 'bg-emerald-50 text-emerald-700', PATIENT: 'bg-slate-100 text-slate-600' }[r] || '' }

// ── Form Modal ──
function openAdd() {
  const typeMap = { users: 'user', depts: 'dept', docs: 'doc', kb: 'kb', announcements: 'announcement' }
  editingType.value = typeMap[activeTab.value] || 'user'
  editingPayload.value = getDefaultPayload(editingType.value)
  formErrors.value = {}
  showFormModal.value = true
}

function openEdit(item) {
  const typeMap = { users: 'user', depts: 'dept', docs: 'doc', kb: 'kb', announcements: 'announcement' }
  editingType.value = typeMap[activeTab.value] || 'user'
  editingPayload.value = { ...item }
  formErrors.value = {}
  showFormModal.value = true
}

function getDefaultPayload(type) {
  const defaults = {
    user: { username: '', password: '', realName: '', phone: '', email: '', role: 'PATIENT', status: 1 },
    dept: { name: '', floor: '', location: '', description: '', phone: '', sortOrder: 0, status: 1 },
    doc: { name: '', title: '', departmentId: null, specialization: '', description: '', consultationLocation: '', status: 1 },
    kb: { question: '', answer: '', category: '', departmentId: null, status: 1 },
    announcement: { title: '', content: '', type: 'NOTICE', status: 0 },
  }
  return defaults[type] || {}
}

function validateForm() {
  const errs = {}
  const p = editingPayload.value
  switch (editingType.value) {
    case 'user': if (!p.username?.trim()) errs.username = '必填'; if (!p.id && !p.password?.trim()) errs.password = '新增时必填'; break
    case 'dept': if (!p.name?.trim()) errs.name = '必填'; break
    case 'doc': if (!p.name?.trim()) errs.name = '必填'; break
    case 'kb': if (!p.question?.trim()) errs.question = '必填'; if (!p.answer?.trim()) errs.answer = '必填'; break
    case 'announcement': if (!p.title?.trim()) errs.title = '必填'; if (!p.content?.trim()) errs.content = '必填'; break
  }
  formErrors.value = errs
  return Object.keys(errs).length === 0
}

async function saveForm() {
  if (!validateForm()) return
  isGlobalLoading.value = true
  try {
    const eps = { user: '/api/user', dept: '/api/department', doc: '/api/doctor', kb: '/api/knowledge-base', announcement: '/api/announcement' }
    const method = editingPayload.value.id ? 'PUT' : 'POST'
    await api(eps[editingType.value], { method, body: JSON.stringify(editingPayload.value) })
    showToast(editingPayload.value.id ? '更新成功' : '新增成功', 'success')
    showFormModal.value = false
    await fetchAll()
  } catch (e) { showToast(e.message || '保存失败', 'error') }
  finally { isGlobalLoading.value = false }
}

// ── Delete Modal ──
function promptDelete(id, label) {
  deleteTarget.value = { id, label }
  isBulkDelete.value = false
  showDeleteModal.value = true
}

function promptBulkDelete() {
  if (!selectedIds.value.length) return
  deleteTarget.value = { ids: selectedIds.value, count: selectedIds.value.length }
  isBulkDelete.value = true
  showDeleteModal.value = true
}

async function confirmDelete() {
  showDeleteModal.value = false
  isGlobalLoading.value = true
  try {
    const eps = {
      users: '/api/user', depts: '/api/department', docs: '/api/doctor',
      kb: '/api/knowledge-base', announcements: '/api/announcement', ratings: '/api/rating'
    }
    if (isBulkDelete.value) {
      await api(`${eps[activeTab.value]}/batch`, { method: 'DELETE', body: JSON.stringify(deleteTarget.value.ids) })
      showToast(`已删除 ${deleteTarget.value.count} 条`, 'success')
    } else {
      await api(`${eps[activeTab.value]}/${deleteTarget.value.id}`, { method: 'DELETE' })
      showToast('已删除', 'success')
    }
    await fetchAll()
  } catch (e) { showToast(e.message || '删除失败', 'error') }
  finally { isGlobalLoading.value = false; deleteTarget.value = null }
}

// ── Actions ──
async function publishAnn(id) { await api(`/api/announcement/publish/${id}`, { method: 'PUT' }); showToast('已发布', 'success'); await fetchAll() }
async function reIndex() { const msg = await api('/api/v2/qa/refresh-index', { method: 'POST' }); showToast(msg, 'success') }

function logout() {
  try { store.logout() } catch {}
  try { sessionStorage.clear() } catch {}
  try { window.location.replace('/login') } catch { window.location.href = '/login' }
}

function switchTab(key) { activeTab.value = key; selectedIds.value = []; searchQuery.value = '' }

onMounted(() => fetchAll())
</script>

<template>
  <div class="min-h-screen bg-gradient-to-br from-slate-50 via-white to-slate-100 text-slate-800">
    <!-- Toast -->
    <Transition name="fade">
      <div v-if="actionMessage"
        :class="['fixed top-5 left-1/2 -translate-x-1/2 z-[60] px-5 py-3 rounded-2xl shadow-2xl flex items-center gap-3 border text-sm font-semibold backdrop-blur-sm',
          actionMessage.type === 'success' ? 'bg-emerald-50/95 border-emerald-200 text-emerald-800' : 'bg-rose-50/95 border-rose-200 text-rose-800']">
        <CheckCircle v-if="actionMessage.type === 'success'" class="w-5 h-5 text-emerald-500" />
        <AlertCircle v-else class="w-5 h-5 text-rose-500" />
        {{ actionMessage.text }}
      </div>
    </Transition>

    <!-- Global Loading -->
    <Transition name="fade">
      <div v-if="isGlobalLoading" class="fixed inset-0 bg-white/70 backdrop-blur-sm z-50 flex items-center justify-center">
        <div class="bg-white p-6 rounded-2xl shadow-2xl flex items-center gap-4 border border-slate-100">
          <RefreshCw class="w-6 h-6 animate-spin text-blue-500" />
          <span class="text-sm font-semibold text-slate-600">处理中...</span>
        </div>
      </div>
    </Transition>

    <!-- Header -->
    <header class="bg-white/90 backdrop-blur-md border-b border-slate-200/60 sticky top-0 z-40 shadow-sm">
      <div class="max-w-7xl mx-auto px-4 py-3 sm:px-6 lg:px-8 flex items-center justify-between">
        <div class="flex items-center gap-3">
          <div class="bg-gradient-to-br from-slate-800 to-slate-900 text-white p-2.5 rounded-xl shadow-lg shadow-slate-800/20">
            <Shield class="w-5 h-5" />
          </div>
          <div>
            <h1 class="text-lg font-bold text-slate-900 tracking-tight">医慧通 · 管理后台</h1>
            <p class="text-[10px] text-slate-400">主数据管理 / 全局控制</p>
          </div>
        </div>
        <div class="flex items-center gap-3">
          <button @click="fetchAll" class="p-2 rounded-xl bg-slate-100 hover:bg-blue-50 text-slate-500 hover:text-blue-600 transition-all" title="刷新">
            <RefreshCw class="w-4 h-4" />
          </button>
          <span class="hidden sm:inline text-xs text-slate-500">{{ user?.realName }}（管理员）</span>
          <button @click="logout" class="text-xs bg-white hover:bg-slate-50 text-slate-500 border border-slate-200 px-3 py-1.5 rounded-lg">退出</button>
        </div>
      </div>
    </header>

    <main class="max-w-7xl mx-auto px-4 py-6 sm:px-6 lg:px-8 space-y-6">
      <!-- Stats -->
      <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-3">
        <div v-for="s in statsCards" :key="s.label"
          class="bg-white rounded-2xl border border-slate-200/60 p-4 shadow-sm hover:shadow-md transition-all cursor-pointer"
          @click="switchTab(s.label === '用户总数' ? 'users' : s.label === '科室数量' ? 'depts' : s.label === '医生数量' ? 'docs' : s.label === '知识库' ? 'kb' : s.label === '公告' ? 'announcements' : 'ratings')">
          <div class="flex items-center justify-between mb-2">
            <span class="text-[10px] text-slate-400 font-bold uppercase tracking-wide">{{ s.label }}</span>
            <component :is="s.icon" :class="['w-4 h-4',
              s.color === 'blue' ? 'text-blue-500' : s.color === 'emerald' ? 'text-emerald-500' :
              s.color === 'indigo' ? 'text-indigo-500' : s.color === 'purple' ? 'text-purple-500' :
              s.color === 'amber' ? 'text-amber-500' : 'text-rose-500']" />
          </div>
          <p class="text-2xl font-bold text-slate-800">{{ s.value }} <span class="text-xs text-slate-400 font-normal">{{ s.unit }}</span></p>
        </div>
      </div>

      <!-- Main CRUD Panel -->
      <section class="bg-white rounded-2xl border border-slate-200/60 shadow-sm overflow-hidden">
        <!-- Top Bar -->
        <div class="bg-gradient-to-r from-slate-900 to-slate-800 px-5 py-3.5 flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3">
          <h2 class="text-sm font-extrabold text-white uppercase tracking-wider flex items-center gap-2">
            <Settings class="w-4 h-4 text-blue-400" /> 主数据管理
          </h2>
          <!-- Tab Pills -->
          <div class="flex gap-1 bg-slate-800 p-1 rounded-xl flex-wrap">
            <button v-for="t in tabs" :key="t.key" @click="switchTab(t.key)"
              :class="['px-3 py-1.5 rounded-lg text-xs font-bold transition-all flex items-center gap-1.5',
                activeTab === t.key ? 'bg-white text-slate-900 shadow-sm' : 'text-slate-400 hover:text-white']">
              <component :is="t.icon" class="w-3 h-3" />
              {{ t.label }}
            </button>
          </div>
        </div>

        <!-- Toolbar -->
        <div class="px-4 py-3 bg-slate-50/80 border-b border-slate-200 flex flex-wrap items-center justify-between gap-3">
          <div class="flex items-center gap-3">
            <div class="relative">
              <Search class="w-3.5 h-3.5 text-slate-400 absolute left-3 top-1/2 -translate-y-1/2" />
              <input v-model="searchQuery" placeholder="搜索..." class="pl-8 pr-3 py-1.5 bg-white border border-slate-200 rounded-lg text-xs outline-none focus:border-blue-400 w-40 sm:w-56" />
            </div>
            <span class="text-[10px] text-slate-400 font-medium">
              已选 <strong class="text-slate-600">{{ selectedIds.length }}</strong> 条
            </span>
            <button @click="promptBulkDelete" :disabled="!selectedIds.length"
              class="bg-rose-50 hover:bg-rose-100 text-rose-600 disabled:bg-slate-100 disabled:text-slate-400 border border-rose-200 disabled:border-slate-200 px-3 py-1 rounded-lg font-bold text-[10px] disabled:cursor-not-allowed transition-all flex items-center gap-1">
              <Trash2 class="w-3 h-3" /> 批量删除
            </button>
          </div>
          <div class="flex items-center gap-2">
            <button v-if="activeTab === 'kb'" @click="reIndex"
              class="bg-purple-50 hover:bg-purple-100 text-purple-700 border border-purple-200 px-3 py-1.5 rounded-lg font-bold text-[10px] flex items-center gap-1 transition-all">
              <RefreshCw class="w-3 h-3" /> 刷新RAG索引
            </button>
            <button v-if="activeTab !== 'ratings'" @click="openAdd"
              class="bg-blue-600 hover:bg-blue-700 text-white px-4 py-1.5 rounded-lg font-bold text-[10px] shadow-sm shadow-blue-500/20 transition-all flex items-center gap-1.5">
              <Plus class="w-3.5 h-3.5" /> 新增
            </button>
          </div>
        </div>

        <!-- Data Tables -->
        <div class="overflow-x-auto">
          <!-- ═══ USERS ═══ -->
          <table v-if="activeTab === 'users'" class="min-w-full divide-y divide-slate-100 text-xs">
            <thead class="bg-slate-50 text-slate-400 font-bold text-[10px] uppercase tracking-wider">
              <tr><th class="px-4 py-3 w-10"><input type="checkbox" v-model="toggleAll" class="rounded" /></th>
                <th class="px-4 py-3 text-left">用户名</th><th class="px-4 py-3 text-left">姓名</th>
                <th class="px-4 py-3 text-left hidden md:table-cell">电话</th><th class="px-4 py-3 text-left hidden lg:table-cell">邮箱</th>
                <th class="px-4 py-3 text-left">角色</th><th class="px-4 py-3 text-center">状态</th><th class="px-4 py-3 text-right">操作</th></tr>
            </thead>
            <tbody class="divide-y divide-slate-100">
              <tr v-for="u in getList()" :key="u.id" class="hover:bg-blue-50/30 transition-colors">
                <td class="px-4 py-3"><input type="checkbox" :value="u.id" v-model="selectedIds" class="rounded" /></td>
                <td class="px-4 py-3 font-semibold text-slate-800">{{ u.username }}</td>
                <td class="px-4 py-3">{{ u.realName || '-' }}</td>
                <td class="px-4 py-3 font-mono text-slate-500 hidden md:table-cell">{{ u.phone || '-' }}</td>
                <td class="px-4 py-3 text-slate-400 hidden lg:table-cell">{{ u.email || '-' }}</td>
                <td class="px-4 py-3"><span :class="['px-2 py-0.5 rounded-full text-[9px] font-bold', getRoleColor(u.role)]">{{ getRoleLabel(u.role) }}</span></td>
                <td class="px-4 py-3 text-center"><span :class="['inline-flex items-center gap-1 px-2 py-0.5 rounded-full text-[9px] font-bold', u.status === 1 ? 'bg-emerald-50 text-emerald-700' : 'bg-slate-100 text-slate-500']"><span :class="['h-1.5 w-1.5 rounded-full', u.status === 1 ? 'bg-emerald-500' : 'bg-slate-400']"></span>{{ u.status === 1 ? '启用' : '禁用' }}</span></td>
                <td class="px-4 py-3 text-right">
                  <div class="flex items-center justify-end gap-1">
                    <button @click="openEdit(u)" class="p-1.5 rounded-lg hover:bg-blue-100 text-blue-600 transition-all" title="编辑"><Edit3 class="w-3.5 h-3.5" /></button>
                    <button @click="promptDelete(u.id, u.username)" class="p-1.5 rounded-lg hover:bg-rose-100 text-rose-500 transition-all" title="删除"><Trash2 class="w-3.5 h-3.5" /></button>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>

          <!-- ═══ DEPARTMENTS ═══ -->
          <table v-if="activeTab === 'depts'" class="min-w-full divide-y divide-slate-100 text-xs">
            <thead class="bg-slate-50 text-slate-400 font-bold text-[10px] uppercase tracking-wider">
              <tr><th class="px-4 py-3 w-10"><input type="checkbox" v-model="toggleAll" class="rounded" /></th>
                <th class="px-4 py-3 text-left">名称</th><th class="px-4 py-3 text-left">楼层</th>
                <th class="px-4 py-3 text-left hidden md:table-cell">位置</th><th class="px-4 py-3 text-left hidden lg:table-cell">电话</th>
                <th class="px-4 py-3 text-center">排序</th><th class="px-4 py-3 text-center">状态</th><th class="px-4 py-3 text-right">操作</th></tr>
            </thead>
            <tbody class="divide-y divide-slate-100">
              <tr v-for="d in getList()" :key="d.id" class="hover:bg-blue-50/30 transition-colors">
                <td class="px-4 py-3"><input type="checkbox" :value="d.id" v-model="selectedIds" class="rounded" /></td>
                <td class="px-4 py-3 font-bold text-slate-800">{{ d.name }}</td>
                <td class="px-4 py-3">{{ d.floor || '-' }}</td>
                <td class="px-4 py-3 text-slate-500 hidden md:table-cell max-w-xs truncate">{{ d.location || '-' }}</td>
                <td class="px-4 py-3 font-mono text-slate-500 hidden lg:table-cell">{{ d.phone || '-' }}</td>
                <td class="px-4 py-3 text-center font-bold">{{ d.sortOrder }}</td>
                <td class="px-4 py-3 text-center"><span :class="['inline-flex items-center gap-1 px-2 py-0.5 rounded-full text-[9px] font-bold', d.status === 1 ? 'bg-emerald-50 text-emerald-700' : 'bg-slate-100 text-slate-500']"><span :class="['h-1.5 w-1.5 rounded-full', d.status === 1 ? 'bg-emerald-500' : 'bg-slate-400']"></span>{{ d.status === 1 ? '启用' : '禁用' }}</span></td>
                <td class="px-4 py-3 text-right">
                  <div class="flex items-center justify-end gap-1">
                    <button @click="openEdit(d)" class="p-1.5 rounded-lg hover:bg-blue-100 text-blue-600 transition-all" title="编辑"><Edit3 class="w-3.5 h-3.5" /></button>
                    <button @click="promptDelete(d.id, d.name)" class="p-1.5 rounded-lg hover:bg-rose-100 text-rose-500 transition-all" title="删除"><Trash2 class="w-3.5 h-3.5" /></button>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>

          <!-- ═══ DOCTORS ═══ -->
          <table v-if="activeTab === 'docs'" class="min-w-full divide-y divide-slate-100 text-xs">
            <thead class="bg-slate-50 text-slate-400 font-bold text-[10px] uppercase tracking-wider">
              <tr><th class="px-4 py-3 w-10"><input type="checkbox" v-model="toggleAll" class="rounded" /></th>
                <th class="px-4 py-3 text-left">姓名</th><th class="px-4 py-3 text-left">职称</th>
                <th class="px-4 py-3 text-left">科室</th><th class="px-4 py-3 text-left hidden lg:table-cell">专长</th>
                <th class="px-4 py-3 text-center">状态</th><th class="px-4 py-3 text-right">操作</th></tr>
            </thead>
            <tbody class="divide-y divide-slate-100">
              <tr v-for="d in getList()" :key="d.id" class="hover:bg-blue-50/30 transition-colors">
                <td class="px-4 py-3"><input type="checkbox" :value="d.id" v-model="selectedIds" class="rounded" /></td>
                <td class="px-4 py-3 font-bold text-slate-800">{{ d.name }}</td>
                <td class="px-4 py-3">{{ d.title || '-' }}</td>
                <td class="px-4 py-3"><span class="bg-indigo-50 text-indigo-700 px-2 py-0.5 rounded-md text-[10px] font-bold">{{ getDeptName(d.departmentId) }}</span></td>
                <td class="px-4 py-3 text-slate-500 max-w-xs truncate hidden lg:table-cell">{{ d.specialization || '-' }}</td>
                <td class="px-4 py-3 text-center"><span :class="['inline-flex items-center gap-1 px-2 py-0.5 rounded-full text-[9px] font-bold', d.status === 1 ? 'bg-emerald-50 text-emerald-700' : 'bg-amber-50 text-amber-700']"><span :class="['h-1.5 w-1.5 rounded-full', d.status === 1 ? 'bg-emerald-500 animate-pulse' : 'bg-amber-500']"></span>{{ d.status === 1 ? '在岗' : '休假' }}</span></td>
                <td class="px-4 py-3 text-right">
                  <div class="flex items-center justify-end gap-1">
                    <button @click="openEdit(d)" class="p-1.5 rounded-lg hover:bg-blue-100 text-blue-600 transition-all" title="编辑"><Edit3 class="w-3.5 h-3.5" /></button>
                    <button @click="promptDelete(d.id, d.name)" class="p-1.5 rounded-lg hover:bg-rose-100 text-rose-500 transition-all" title="删除"><Trash2 class="w-3.5 h-3.5" /></button>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>

          <!-- ═══ KNOWLEDGE BASE ═══ -->
          <table v-if="activeTab === 'kb'" class="min-w-full divide-y divide-slate-100 text-xs">
            <thead class="bg-slate-50 text-slate-400 font-bold text-[10px] uppercase tracking-wider">
              <tr><th class="px-4 py-3 w-10"><input type="checkbox" v-model="toggleAll" class="rounded" /></th>
                <th class="px-4 py-3 text-left">问题</th><th class="px-4 py-3 text-left hidden lg:table-cell">答案</th>
                <th class="px-4 py-3 text-left">分类</th><th class="px-4 py-3 text-center">浏览</th>
                <th class="px-4 py-3 text-center">状态</th><th class="px-4 py-3 text-right">操作</th></tr>
            </thead>
            <tbody class="divide-y divide-slate-100">
              <tr v-for="k in getList()" :key="k.id" class="hover:bg-blue-50/30 transition-colors">
                <td class="px-4 py-3"><input type="checkbox" :value="k.id" v-model="selectedIds" class="rounded" /></td>
                <td class="px-4 py-3 font-semibold text-slate-800 max-w-sm truncate">{{ k.question }}</td>
                <td class="px-4 py-3 text-slate-500 max-w-xs truncate hidden lg:table-cell">{{ k.answer }}</td>
                <td class="px-4 py-3"><span class="bg-purple-50 text-purple-700 px-2 py-0.5 rounded-md text-[10px] font-bold">{{ k.category || '未分类' }}</span></td>
                <td class="px-4 py-3 text-center font-mono font-bold text-slate-500">{{ k.viewCount || 0 }}</td>
                <td class="px-4 py-3 text-center"><span :class="['inline-flex items-center gap-1 px-2 py-0.5 rounded-full text-[9px] font-bold', k.status === 1 ? 'bg-emerald-50 text-emerald-700' : 'bg-slate-100 text-slate-500']"><span :class="['h-1.5 w-1.5 rounded-full', k.status === 1 ? 'bg-emerald-500' : 'bg-slate-400']"></span>{{ k.status === 1 ? '已发布' : '草稿' }}</span></td>
                <td class="px-4 py-3 text-right">
                  <div class="flex items-center justify-end gap-1">
                    <button @click="openEdit(k)" class="p-1.5 rounded-lg hover:bg-blue-100 text-blue-600 transition-all" title="编辑"><Edit3 class="w-3.5 h-3.5" /></button>
                    <button @click="promptDelete(k.id, k.question)" class="p-1.5 rounded-lg hover:bg-rose-100 text-rose-500 transition-all" title="删除"><Trash2 class="w-3.5 h-3.5" /></button>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>

          <!-- ═══ ANNOUNCEMENTS ═══ -->
          <table v-if="activeTab === 'announcements'" class="min-w-full divide-y divide-slate-100 text-xs">
            <thead class="bg-slate-50 text-slate-400 font-bold text-[10px] uppercase tracking-wider">
              <tr><th class="px-4 py-3 w-10"><input type="checkbox" v-model="toggleAll" class="rounded" /></th>
                <th class="px-4 py-3 text-left">标题</th><th class="px-4 py-3 text-left">类型</th>
                <th class="px-4 py-3 text-center">状态</th><th class="px-4 py-3 text-left hidden md:table-cell">发布时间</th>
                <th class="px-4 py-3 text-right">操作</th></tr>
            </thead>
            <tbody class="divide-y divide-slate-100">
              <tr v-for="a in getList()" :key="a.id" class="hover:bg-blue-50/30 transition-colors">
                <td class="px-4 py-3"><input type="checkbox" :value="a.id" v-model="selectedIds" class="rounded" /></td>
                <td class="px-4 py-3 font-semibold text-slate-800 max-w-sm truncate">{{ a.title }}</td>
                <td class="px-4 py-3"><span :class="['px-2 py-0.5 rounded-md text-[10px] font-bold', a.type === 'NOTICE' ? 'bg-blue-50 text-blue-700' : 'bg-amber-50 text-amber-700']">{{ a.type === 'NOTICE' ? '通知' : '活动' }}</span></td>
                <td class="px-4 py-3 text-center"><span :class="['inline-flex items-center gap-1 px-2 py-0.5 rounded-full text-[9px] font-bold', a.status === 1 ? 'bg-emerald-50 text-emerald-700' : 'bg-amber-50 text-amber-700']"><span :class="['h-1.5 w-1.5 rounded-full', a.status === 1 ? 'bg-emerald-500' : 'bg-amber-500']"></span>{{ a.status === 1 ? '已发布' : '草稿' }}</span></td>
                <td class="px-4 py-3 text-slate-400 hidden md:table-cell">{{ a.publishTime || '未发布' }}</td>
                <td class="px-4 py-3 text-right">
                  <div class="flex items-center justify-end gap-1">
                    <button @click="openEdit(a)" class="p-1.5 rounded-lg hover:bg-blue-100 text-blue-600 transition-all" title="编辑"><Edit3 class="w-3.5 h-3.5" /></button>
                    <button v-if="a.status !== 1" @click="publishAnn(a.id)" class="p-1.5 rounded-lg hover:bg-emerald-100 text-emerald-600 transition-all" title="发布"><Eye class="w-3.5 h-3.5" /></button>
                    <button @click="promptDelete(a.id, a.title)" class="p-1.5 rounded-lg hover:bg-rose-100 text-rose-500 transition-all" title="删除"><Trash2 class="w-3.5 h-3.5" /></button>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>

          <!-- ═══ RATINGS ═══ -->
          <table v-if="activeTab === 'ratings'" class="min-w-full divide-y divide-slate-100 text-xs">
            <thead class="bg-slate-50 text-slate-400 font-bold text-[10px] uppercase tracking-wider">
              <tr><th class="px-4 py-3 w-10"><input type="checkbox" v-model="toggleAll" class="rounded" /></th>
                <th class="px-4 py-3 text-left">用户</th><th class="px-4 py-3 text-left hidden lg:table-cell">会话</th>
                <th class="px-4 py-3 text-center">评分</th><th class="px-4 py-3 text-left">评价</th>
                <th class="px-4 py-3 text-left hidden md:table-cell">时间</th><th class="px-4 py-3 text-right">操作</th></tr>
            </thead>
            <tbody class="divide-y divide-slate-100">
              <tr v-for="r in getList()" :key="r.id" class="hover:bg-blue-50/30 transition-colors">
                <td class="px-4 py-3"><input type="checkbox" :value="r.id" v-model="selectedIds" class="rounded" /></td>
                <td class="px-4 py-3 font-semibold text-slate-800">{{ getUserName(r.userId) }}</td>
                <td class="px-4 py-3 text-slate-400 font-mono hidden lg:table-cell">#{{ String(r.conversationId).slice(-8) }}</td>
                <td class="px-4 py-3 text-center"><span class="text-amber-500 text-sm font-bold">{{ '★'.repeat(r.score) }}{{ '☆'.repeat(5 - r.score) }}</span></td>
                <td class="px-4 py-3 text-slate-500 max-w-xs truncate">{{ r.comment || '-' }}</td>
                <td class="px-4 py-3 text-slate-400 hidden md:table-cell">{{ r.createTime?.slice(0, 10) }}</td>
                <td class="px-4 py-3 text-right">
                  <button @click="promptDelete(r.id, `评价#${String(r.id).slice(-6)}`)" class="p-1.5 rounded-lg hover:bg-rose-100 text-rose-500 transition-all" title="删除"><Trash2 class="w-3.5 h-3.5" /></button>
                </td>
              </tr>
            </tbody>
          </table>
        </div>

        <!-- Empty State -->
        <div v-if="getList().length === 0" class="py-20 text-center">
          <div class="h-14 w-14 mx-auto mb-3 rounded-2xl bg-slate-100 flex items-center justify-center">
            <Search class="w-7 h-7 text-slate-300" />
          </div>
          <p class="text-slate-400 font-semibold text-sm">暂无数据</p>
          <p class="text-xs text-slate-300 mt-1">{{ searchQuery ? '未找到匹配结果' : '点击"新增"添加' }}</p>
        </div>
      </section>
    </main>

    <!-- ═══════════ FORM MODAL ═══════════ -->
    <Transition name="fade">
      <div v-if="showFormModal" class="fixed inset-0 bg-black/40 backdrop-blur-sm z-50 flex items-start justify-center pt-[10vh] px-4" @click.self="showFormModal = false">
        <div class="bg-white rounded-2xl shadow-2xl w-full max-w-xl max-h-[80vh] overflow-y-auto">
          <!-- Modal Header -->
          <div class="sticky top-0 bg-white border-b border-slate-100 px-6 py-4 flex items-center justify-between rounded-t-2xl z-10">
            <div class="flex items-center gap-3">
              <div :class="['h-9 w-9 rounded-xl flex items-center justify-center',
                editingType === 'user' ? 'bg-blue-100 text-blue-600' :
                editingType === 'dept' ? 'bg-emerald-100 text-emerald-600' :
                editingType === 'doc' ? 'bg-indigo-100 text-indigo-600' :
                editingType === 'kb' ? 'bg-purple-100 text-purple-600' :
                'bg-amber-100 text-amber-600']">
                <component :is="editingType === 'user' ? Users : editingType === 'dept' ? Building2 : editingType === 'doc' ? Stethoscope : editingType === 'kb' ? BookOpen : Bell" class="w-4.5 h-4.5" />
              </div>
              <div>
                <h3 class="text-base font-bold text-slate-900">
                  {{ editingPayload.id ? '编辑' : '新增' }}
                  {{ editingType === 'user' ? '用户' : editingType === 'dept' ? '科室' : editingType === 'doc' ? '医生' : editingType === 'kb' ? '知识库条目' : '公告' }}
                </h3>
                <p class="text-[10px] text-slate-400">{{ editingPayload.id ? '修改后点击保存即可更新' : '请填写以下信息' }}</p>
              </div>
            </div>
            <button @click="showFormModal = false" class="p-2 rounded-xl hover:bg-slate-100 text-slate-400 transition-all"><X class="w-5 h-5" /></button>
          </div>

          <!-- Form Body -->
          <div class="p-6 space-y-4">
            <!-- ═══ USER FORM ═══ -->
            <template v-if="editingType === 'user'">
              <div class="grid grid-cols-2 gap-4">
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">用户名 <span class="text-rose-500">*</span></label><input v-model="editingPayload.username" :class="['w-full border rounded-xl px-3 py-2 text-sm outline-none transition-all', formErrors.username ? 'border-rose-300 bg-rose-50' : 'bg-slate-50 border-slate-200 focus:border-blue-400 focus:bg-white']" placeholder="登录用户名" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">密码 <span v-if="!editingPayload.id" class="text-rose-500">*</span><span v-else class="text-slate-300">(留空不修改)</span></label><input v-model="editingPayload.password" type="password" :class="['w-full border rounded-xl px-3 py-2 text-sm outline-none transition-all', formErrors.password ? 'border-rose-300 bg-rose-50' : 'bg-slate-50 border-slate-200 focus:border-blue-400 focus:bg-white']" :placeholder="editingPayload.id ? '留空则不修改密码' : '设置登录密码'" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">真实姓名</label><input v-model="editingPayload.realName" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="真实姓名" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">手机号</label><input v-model="editingPayload.phone" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="手机号" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">邮箱</label><input v-model="editingPayload.email" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="邮箱地址" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">角色</label><select v-model="editingPayload.role" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 transition-all"><option value="PATIENT">患者</option><option value="CS_STAFF">客服</option><option value="DOCTOR">医生</option><option value="ADMIN">管理员</option></select></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">状态</label><div class="flex gap-2 mt-1"><button @click="editingPayload.status = 1" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 1 ? 'bg-emerald-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">启用</button><button @click="editingPayload.status = 0" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 0 ? 'bg-slate-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">禁用</button></div></div>
              </div>
            </template>

            <!-- ═══ DEPT FORM ═══ -->
            <template v-if="editingType === 'dept'">
              <div class="grid grid-cols-2 gap-4">
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">名称 <span class="text-rose-500">*</span></label><input v-model="editingPayload.name" :class="['w-full border rounded-xl px-3 py-2 text-sm outline-none transition-all', formErrors.name ? 'border-rose-300 bg-rose-50' : 'bg-slate-50 border-slate-200 focus:border-blue-400 focus:bg-white']" placeholder="科室名称" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">楼层</label><input v-model="editingPayload.floor" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="如：3层" /></div>
                <div class="col-span-2"><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">位置描述</label><input v-model="editingPayload.location" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="如：门诊楼3层东侧A区" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">电话</label><input v-model="editingPayload.phone" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="科室电话" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">排序</label><input v-model="editingPayload.sortOrder" type="number" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" /></div>
                <div class="col-span-2"><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">描述</label><textarea v-model="editingPayload.description" rows="2" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="科室简介"></textarea></div>
                <div class="col-span-2"><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">状态</label><div class="flex gap-2"><button @click="editingPayload.status = 1" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 1 ? 'bg-emerald-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">启用</button><button @click="editingPayload.status = 0" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 0 ? 'bg-slate-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">禁用</button></div></div>
              </div>
            </template>

            <!-- ═══ DOCTOR FORM ═══ -->
            <template v-if="editingType === 'doc'">
              <div class="grid grid-cols-2 gap-4">
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">姓名 <span class="text-rose-500">*</span></label><input v-model="editingPayload.name" :class="['w-full border rounded-xl px-3 py-2 text-sm outline-none transition-all', formErrors.name ? 'border-rose-300 bg-rose-50' : 'bg-slate-50 border-slate-200 focus:border-blue-400 focus:bg-white']" placeholder="医生姓名" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">职称</label><input v-model="editingPayload.title" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="如：主任医师" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">所属科室</label><select v-model="editingPayload.departmentId" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 transition-all"><option :value="null">-- 请选择 --</option><option v-for="d in adminDepts" :key="d.id" :value="d.id">{{ d.name }}</option></select></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">问诊地点</label><input v-model="editingPayload.consultationLocation" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="如：3层呼吸内科A区3诊室" /></div>
                <div class="col-span-2"><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">专长</label><input v-model="editingPayload.specialization" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="如：冠心病、高血压、心律失常" /></div>
                <div class="col-span-2"><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">简介</label><textarea v-model="editingPayload.description" rows="2" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="医生简介"></textarea></div>
                <div class="col-span-2"><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">状态</label><div class="flex gap-2"><button @click="editingPayload.status = 1" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 1 ? 'bg-emerald-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">在岗</button><button @click="editingPayload.status = 0" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 0 ? 'bg-slate-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">休假</button></div></div>
              </div>
            </template>

            <!-- ═══ KB FORM ═══ -->
            <template v-if="editingType === 'kb'">
              <div class="space-y-4">
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">问题 <span class="text-rose-500">*</span></label><input v-model="editingPayload.question" :class="['w-full border rounded-xl px-3 py-2 text-sm outline-none transition-all', formErrors.question ? 'border-rose-300 bg-rose-50' : 'bg-slate-50 border-slate-200 focus:border-blue-400 focus:bg-white']" placeholder="常见问题" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">答案 <span class="text-rose-500">*</span></label><textarea v-model="editingPayload.answer" rows="5" :class="['w-full border rounded-xl px-3 py-2 text-sm outline-none transition-all', formErrors.answer ? 'border-rose-300 bg-rose-50' : 'bg-slate-50 border-slate-200 focus:border-blue-400 focus:bg-white']" placeholder="答案内容"></textarea></div>
                <div class="grid grid-cols-2 gap-4">
                  <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">分类</label><input v-model="editingPayload.category" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 focus:bg-white transition-all" placeholder="如：挂号指引" /></div>
                  <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">关联科室</label><select v-model="editingPayload.departmentId" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 transition-all"><option :value="null">-- 无 --</option><option v-for="d in adminDepts" :key="d.id" :value="d.id">{{ d.name }}</option></select></div>
                  <div class="col-span-2"><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">状态</label><div class="flex gap-2"><button @click="editingPayload.status = 1" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 1 ? 'bg-emerald-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">已发布</button><button @click="editingPayload.status = 0" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 0 ? 'bg-slate-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">草稿</button></div></div>
                </div>
              </div>
            </template>

            <!-- ═══ ANNOUNCEMENT FORM ═══ -->
            <template v-if="editingType === 'announcement'">
              <div class="space-y-4">
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">标题 <span class="text-rose-500">*</span></label><input v-model="editingPayload.title" :class="['w-full border rounded-xl px-3 py-2 text-sm outline-none transition-all', formErrors.title ? 'border-rose-300 bg-rose-50' : 'bg-slate-50 border-slate-200 focus:border-blue-400 focus:bg-white']" placeholder="公告标题" /></div>
                <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">内容 <span class="text-rose-500">*</span></label><textarea v-model="editingPayload.content" rows="5" :class="['w-full border rounded-xl px-3 py-2 text-sm outline-none transition-all', formErrors.content ? 'border-rose-300 bg-rose-50' : 'bg-slate-50 border-slate-200 focus:border-blue-400 focus:bg-white']" placeholder="公告内容"></textarea></div>
                <div class="grid grid-cols-2 gap-4">
                  <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">类型</label><select v-model="editingPayload.type" class="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm outline-none focus:border-blue-400 transition-all"><option value="NOTICE">停诊通知</option><option value="ACTIVITY">活动公告</option></select></div>
                  <div><label class="block text-[10px] font-bold text-slate-500 uppercase tracking-wide mb-1.5">状态</label><div class="flex gap-2"><button @click="editingPayload.status = 1" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 1 ? 'bg-emerald-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">已发布</button><button @click="editingPayload.status = 0" :class="['flex-1 py-2 rounded-xl text-xs font-bold transition-all', editingPayload.status === 0 ? 'bg-slate-500 text-white shadow-md' : 'bg-slate-100 text-slate-500']">草稿</button></div></div>
                </div>
              </div>
            </template>
          </div>

          <!-- Modal Footer -->
          <div class="sticky bottom-0 bg-white border-t border-slate-100 px-6 py-4 flex gap-3 justify-end rounded-b-2xl">
            <button @click="showFormModal = false" class="px-5 py-2.5 text-sm text-slate-500 hover:bg-slate-50 rounded-xl transition-all font-medium">取消</button>
            <button @click="saveForm" class="px-6 py-2.5 bg-blue-600 hover:bg-blue-700 text-white rounded-xl text-sm font-bold shadow-md shadow-blue-500/20 transition-all flex items-center gap-2">
              <CheckCircle class="w-4 h-4" /> 保存
            </button>
          </div>
        </div>
      </div>
    </Transition>

    <!-- ═══════════ DELETE CONFIRMATION MODAL ═══════════ -->
    <Transition name="fade">
      <div v-if="showDeleteModal" class="fixed inset-0 bg-black/40 backdrop-blur-sm z-50 flex items-center justify-center px-4" @click.self="showDeleteModal = false">
        <div class="bg-white rounded-2xl p-6 shadow-2xl max-w-md w-full">
          <div class="flex items-start gap-4 mb-6">
            <div class="h-12 w-12 rounded-2xl bg-rose-100 flex items-center justify-center shrink-0">
              <AlertCircle class="w-6 h-6 text-rose-500" />
            </div>
            <div>
              <h3 class="text-lg font-bold text-slate-900">{{ isBulkDelete ? '批量删除' : '确认删除' }}</h3>
              <p class="text-sm text-slate-500 mt-1">
                {{ isBulkDelete
                  ? `确定删除选中的 ${deleteTarget?.count} 条记录吗？此操作不可撤销。`
                  : `确定删除「${deleteTarget?.label}」吗？此操作不可撤销。`
                }}
              </p>
            </div>
          </div>
          <div class="flex gap-3 justify-end">
            <button @click="showDeleteModal = false" class="px-5 py-2.5 text-sm text-slate-500 hover:bg-slate-50 rounded-xl transition-all font-medium">取消</button>
            <button @click="confirmDelete" class="px-6 py-2.5 bg-rose-600 hover:bg-rose-700 text-white rounded-xl text-sm font-bold shadow-md shadow-rose-500/20 transition-all flex items-center gap-2">
              <Trash2 class="w-4 h-4" /> 确认删除
            </button>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<style scoped>
.fade-enter-active, .fade-leave-active { transition: opacity 0.2s ease; }
.fade-enter-from, .fade-leave-to { opacity: 0; }
</style>
