<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../../stores/user'
import {
  Stethoscope, Calendar, Clock, CheckCircle, XCircle,
  RefreshCw, Users, UserCheck, AlertCircle, ChevronDown,
  ChevronUp, Search, Filter, Activity, ArrowUpRight,
  MapPin, Award, BookOpen
} from 'lucide-vue-next'
import { request as api } from '../../api'

const store = useUserStore()
const router = useRouter()
const user = store.user

// ── State ──
const allAppts = ref([])
const adminDepts = ref([])
const userMap = ref({})
const doctorOnDuty = ref(true)
const doctorId = ref(null)
const doctorRecord = ref(null)
const actionMessage = ref(null)
const isGlobalLoading = ref(false)
const activeTab = ref('PENDING')       // ALL | PENDING | CONFIRMED | CANCELLED
const expandedGroups = ref({ today: true, upcoming: true, past: true })
const showRejectConfirm = ref(null)     // id of appointment being rejected
let pollTimer = null

// ── Toast ──
function showToast(text, type = 'success') {
  actionMessage.value = { text, type }
  setTimeout(() => { if (actionMessage.value?.text === text) actionMessage.value = null }, 3500)
}

// ── Data Fetching ──
async function fetchData() {
  isGlobalLoading.value = true
  try {
    await fetchCore()
  } catch (e) {
    console.error('Fetch error:', e)
  } finally {
    isGlobalLoading.value = false
  }
}

/** Core fetch logic — used by both manual refresh and silent poll */
async function fetchCore() {
  // 1. Find doctor record
  const allDocs = await api('/api/doctor/list?pageSize=200')
  const doc = (allDocs.records || []).find(d => d.name === (user.realName || user.username))
  if (doc) {
    doctorRecord.value = doc
    doctorId.value = doc.id
    doctorOnDuty.value = doc.status === 1
  }

  // 2. Fetch departments
  const deps = await api('/api/department/all')
  adminDepts.value = deps || []

  // 3. Fetch appointments
  if (doctorId.value) {
    const appts = await api(`/api/appointment/list?doctorId=${doctorId.value}&pageSize=500`)
    allAppts.value = appts.records || []

    // 4. Fetch user info for all patient IDs
    const userIds = [...new Set(allAppts.value.map(a => a.userId))]
    await fetchUsers(userIds)
  }
}

/** Silent background poll — no loading spinner, no error noise */
async function silentPoll() {
  try {
    if (!doctorId.value) return
    const appts = await api(`/api/appointment/list?doctorId=${doctorId.value}&pageSize=500`)
    allAppts.value = appts.records || []
    const userIds = [...new Set(allAppts.value.map(a => a.userId))]
    await fetchUsers(userIds)
  } catch { /* silent */ }
}

async function fetchUsers(userIds) {
  try {
    const allUsers = await api('/api/user/list?pageSize=500')
    const records = allUsers.records || []
    for (const uid of userIds) {
      const u = records.find(r => r.id === uid)
      if (u) userMap.value[uid] = u
    }
  } catch { /* silent */ }
}

// ── Computed ──
const stats = computed(() => {
  const total = allAppts.value.length
  const pending = allAppts.value.filter(a => a.status === 'PENDING').length
  const confirmed = allAppts.value.filter(a => a.status === 'CONFIRMED').length
  const today = new Date().toISOString().split('T')[0]
  const todayCount = allAppts.value.filter(a => a.appointmentDate === today && a.status !== 'CANCELLED').length
  return { total, pending, confirmed, todayCount }
})

const filteredAppts = computed(() => {
  if (activeTab.value === 'ALL') return allAppts.value
  return allAppts.value.filter(a => a.status === activeTab.value)
})

const groupedAppts = computed(() => {
  const today = new Date().toISOString().split('T')[0]
  const now = new Date()

  const groups = { today: [], upcoming: [], past: [] }

  for (const a of filteredAppts.value) {
    if (a.appointmentDate === today) {
      groups.today.push(a)
    } else if (a.appointmentDate > today) {
      groups.upcoming.push(a)
    } else {
      // Past: check if it's today but time has passed, or earlier date
      if (a.appointmentDate < today) {
        groups.past.push(a)
      } else {
        // Same day but check time
        const [h, m] = (a.endTime || a.startTime).split(':').map(Number)
        const endTime = new Date()
        endTime.setHours(h, m, 0, 0)
        if (endTime < now) {
          groups.past.push(a)
        } else {
          groups.today.push(a)
        }
      }
    }
  }

  // Sort within groups
  for (const key of Object.keys(groups)) {
    groups[key].sort((a, b) => {
      if (a.appointmentDate !== b.appointmentDate) return a.appointmentDate.localeCompare(b.appointmentDate)
      return (a.startTime || '').localeCompare(b.startTime || '')
    })
  }

  return groups
})

// ── Helpers ──
function getDeptName(id) {
  return adminDepts.value.find(d => d.id === id)?.name || `科室#${id}`
}

function getUserName(userId) {
  const u = userMap.value[userId]
  return u ? (u.realName || u.username) : `患者#${userId}`
}

function getUserInfo(userId) {
  const u = userMap.value[userId]
  if (!u) return {}
  return { name: u.realName || u.username, phone: u.phone || '', email: u.email || '' }
}

function formatDate(dateStr) {
  if (!dateStr) return ''
  const d = new Date(dateStr)
  const weekDays = ['周日', '周一', '周二', '周三', '周四', '周五', '周六']
  return `${dateStr} ${weekDays[d.getDay()]}`
}

function formatTime(timeStr) {
  if (!timeStr) return ''
  return timeStr.slice(0, 5)
}

function statusLabel(status) {
  const map = { PENDING: '待确认', CONFIRMED: '已确认', CANCELLED: '已取消' }
  return map[status] || status
}

function hasGroupContent(key) {
  return groupedAppts.value[key] && groupedAppts.value[key].length > 0
}

function groupLabel(key) {
  const map = { today: '📅 今日预约', upcoming: '🔜 即将到来', past: '📋 历史记录' }
  return map[key] || key
}

function groupCount(key) {
  return (groupedAppts.value[key] || []).length
}

// ── Actions ──
async function confirmAppt(appt) {
  if (!doctorId.value) return
  isGlobalLoading.value = true
  try {
    await api(`/api/appointment/${appt.id}/confirm?doctorId=${doctorId.value}`, { method: 'PUT' })
    appt.status = 'CONFIRMED'
    showToast(`预约 APP-${appt.id} 已确认`, 'success')
  } catch (e) {
    showToast(e.message || '确认失败', 'error')
  } finally {
    isGlobalLoading.value = false
  }
}

async function rejectAppt(appt) {
  showRejectConfirm.value = null
  isGlobalLoading.value = true
  try {
    // Cancel as doctor — use the cancel endpoint with a system user approach
    // Since cancel requires userId, we use appt.userId
    await api(`/api/appointment/${appt.id}/cancel?userId=${appt.userId}`, { method: 'PUT' })
    appt.status = 'CANCELLED'
    showToast(`预约 APP-${appt.id} 已取消`, 'info')
  } catch (e) {
    showToast(e.message || '操作失败', 'error')
  } finally {
    isGlobalLoading.value = false
  }
}

async function toggleDuty() {
  if (!doctorRecord.value) return
  isGlobalLoading.value = true
  try {
    const payload = { ...doctorRecord.value, status: doctorOnDuty.value ? 1 : 0 }
    await api('/api/doctor', { method: 'PUT', body: JSON.stringify(payload) })
    doctorRecord.value.status = doctorOnDuty.value ? 1 : 0
    showToast(doctorOnDuty.value ? '已设为在岗 ✅' : '已设为休假 🌙', 'info')
  } catch (e) {
    doctorOnDuty.value = !doctorOnDuty.value
  } finally {
    isGlobalLoading.value = false
  }
}

function toggleGroup(key) {
  expandedGroups.value[key] = !expandedGroups.value[key]
}

function logout() {
  clearInterval(pollTimer)
  pollTimer = null
  try { store.logout() } catch {}
  try { sessionStorage.clear() } catch {}
  try { window.location.replace('/login') } catch { window.location.href = '/login' }
}

// ── Lifecycle ──
onMounted(async () => {
  await fetchData()
  // Poll every 10 seconds for real-time appointment updates
  pollTimer = setInterval(() => silentPoll(), 10000)
})

onUnmounted(() => {
  clearInterval(pollTimer)
  pollTimer = null
})
</script>

<template>
  <div class="min-h-screen bg-gradient-to-br from-slate-50 via-white to-blue-50/30 text-slate-800">
    <!-- Toast -->
    <Transition name="fade">
      <div v-if="actionMessage"
        :class="['fixed top-5 left-1/2 -translate-x-1/2 z-50 px-5 py-3 rounded-2xl shadow-2xl flex items-center gap-3 border text-sm font-semibold backdrop-blur-sm',
          actionMessage.type === 'success' ? 'bg-emerald-50/95 border-emerald-200 text-emerald-800' :
          actionMessage.type === 'error' ? 'bg-rose-50/95 border-rose-200 text-rose-800' :
          'bg-blue-50/95 border-blue-200 text-blue-800']">
        <CheckCircle v-if="actionMessage.type === 'success'" class="w-5 h-5 text-emerald-500" />
        <AlertCircle v-else-if="actionMessage.type === 'error'" class="w-5 h-5 text-rose-500" />
        <Activity v-else class="w-5 h-5 text-blue-500" />
        {{ actionMessage.text }}
      </div>
    </Transition>

    <!-- Global Loading Overlay -->
    <Transition name="fade">
      <div v-if="isGlobalLoading" class="fixed inset-0 bg-white/70 backdrop-blur-sm z-50 flex items-center justify-center">
        <div class="bg-white p-6 rounded-2xl shadow-2xl flex items-center gap-4 border border-slate-100">
          <RefreshCw class="w-6 h-6 animate-spin text-blue-500" />
          <span class="text-sm font-semibold text-slate-600">处理中...</span>
        </div>
      </div>
    </Transition>

    <!-- Header -->
    <header class="bg-white/80 backdrop-blur-md border-b border-slate-200/60 sticky top-0 z-40 shadow-sm">
      <div class="max-w-7xl mx-auto px-4 py-3 sm:px-6 lg:px-8 flex items-center justify-between">
        <!-- Left: Brand -->
        <div class="flex items-center gap-3">
          <div class="bg-gradient-to-br from-blue-600 to-blue-700 text-white p-2.5 rounded-xl shadow-lg shadow-blue-500/20">
            <Stethoscope class="w-5 h-5" />
          </div>
          <div>
            <h1 class="text-lg font-bold text-slate-900 tracking-tight">医慧通 · 医生工作站</h1>
            <p class="text-xs text-slate-400">预约管理 / 患者确认 / 在岗状态</p>
          </div>
        </div>

        <!-- Right: Controls -->
        <div class="flex items-center gap-4">
          <!-- Duty Toggle -->
          <div class="flex items-center gap-2 bg-slate-100 rounded-xl p-1">
            <button @click="doctorOnDuty = true; toggleDuty()"
              :class="['px-3.5 py-1.5 rounded-lg text-xs font-bold transition-all flex items-center gap-1.5',
                doctorOnDuty ? 'bg-emerald-500 text-white shadow-md shadow-emerald-500/20' : 'text-slate-400 hover:text-slate-600']">
              <span class="h-1.5 w-1.5 rounded-full" :class="doctorOnDuty ? 'bg-white animate-pulse' : 'bg-slate-400'"></span>
              在岗
            </button>
            <button @click="doctorOnDuty = false; toggleDuty()"
              :class="['px-3.5 py-1.5 rounded-lg text-xs font-bold transition-all flex items-center gap-1.5',
                !doctorOnDuty ? 'bg-slate-600 text-white shadow-md' : 'text-slate-400 hover:text-slate-600']">
              🌙 休假
            </button>
          </div>

          <!-- User Info -->
          <div class="hidden sm:flex items-center gap-2">
            <div class="h-8 w-8 rounded-full bg-gradient-to-br from-blue-500 to-indigo-600 flex items-center justify-center text-white text-xs font-bold shadow-sm">
              {{ (user?.realName || user?.username || 'D')[0] }}
            </div>
            <div class="text-right">
              <p class="text-xs font-semibold text-slate-700">{{ user?.realName || user?.username }}</p>
              <p class="text-[10px] text-slate-400">{{ doctorRecord?.title || '医生' }}</p>
            </div>
          </div>

          <button @click="fetchData"
            class="p-2 rounded-xl bg-slate-100 hover:bg-blue-50 text-slate-500 hover:text-blue-600 transition-all"
            title="刷新数据">
            <RefreshCw class="w-4 h-4" />
          </button>

          <button @click="logout"
            class="text-xs bg-white hover:bg-slate-50 text-slate-500 border border-slate-200 px-3 py-1.5 rounded-lg transition-colors hover:border-slate-300">
            退出
          </button>
        </div>
      </div>
    </header>

    <!-- Main Content -->
    <main class="max-w-7xl mx-auto px-4 py-6 sm:px-6 lg:px-8 space-y-6">
      <!-- Doctor Profile Card -->
      <div class="bg-white rounded-2xl border border-slate-200/60 shadow-sm overflow-hidden">
        <div class="bg-gradient-to-r from-blue-600 via-blue-700 to-indigo-700 px-6 py-4">
          <div class="flex items-center justify-between">
            <div class="flex items-center gap-4">
              <div class="h-16 w-16 rounded-2xl bg-white/20 backdrop-blur-sm flex items-center justify-center text-white text-2xl font-bold border-2 border-white/30 shadow-inner">
                {{ (user?.realName || user?.username || 'D')[0] }}
              </div>
              <div class="text-white">
                <h2 class="text-xl font-bold tracking-tight">{{ user?.realName || user?.username }}</h2>
                <p class="text-blue-200 text-sm font-medium">{{ doctorRecord?.title || '医生' }}</p>
              </div>
            </div>
            <div :class="['px-4 py-2 rounded-xl text-sm font-bold backdrop-blur-sm border',
              doctorOnDuty ? 'bg-emerald-500/20 border-emerald-400/30 text-emerald-100' : 'bg-slate-500/20 border-slate-400/30 text-slate-300']">
              <span :class="['inline-block h-2 w-2 rounded-full mr-1.5', doctorOnDuty ? 'bg-emerald-300 animate-pulse' : 'bg-slate-400']"></span>
              {{ doctorOnDuty ? '🟢 在岗中' : '🌙 休假中' }}
            </div>
          </div>
        </div>
        <div class="px-6 py-4 grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
          <div class="flex items-center gap-3">
            <div class="h-10 w-10 rounded-xl bg-blue-50 flex items-center justify-center shrink-0">
              <Stethoscope class="w-5 h-5 text-blue-500" />
            </div>
            <div class="min-w-0">
              <p class="text-[10px] text-slate-400 font-medium uppercase tracking-wide">所属科室</p>
              <p class="text-sm font-bold text-slate-800 truncate">{{ getDeptName(doctorRecord?.departmentId) }}</p>
            </div>
          </div>
          <div class="flex items-center gap-3">
            <div class="h-10 w-10 rounded-xl bg-indigo-50 flex items-center justify-center shrink-0">
              <Award class="w-5 h-5 text-indigo-500" />
            </div>
            <div class="min-w-0">
              <p class="text-[10px] text-slate-400 font-medium uppercase tracking-wide">专长领域</p>
              <p class="text-sm font-bold text-slate-800 truncate">{{ doctorRecord?.specialization || '未设置' }}</p>
            </div>
          </div>
          <div class="flex items-center gap-3">
            <div class="h-10 w-10 rounded-xl bg-amber-50 flex items-center justify-center shrink-0">
              <BookOpen class="w-5 h-5 text-amber-500" />
            </div>
            <div class="min-w-0">
              <p class="text-[10px] text-slate-400 font-medium uppercase tracking-wide">简介</p>
              <p class="text-sm font-bold text-slate-800 truncate">{{ doctorRecord?.description || '暂无简介' }}</p>
            </div>
          </div>
          <div class="flex items-center gap-3">
            <div class="h-10 w-10 rounded-xl bg-emerald-50 flex items-center justify-center shrink-0">
              <MapPin class="w-5 h-5 text-emerald-500" />
            </div>
            <div class="min-w-0">
              <p class="text-[10px] text-slate-400 font-medium uppercase tracking-wide">问诊地点</p>
              <p class="text-sm font-bold text-slate-800 truncate">{{ doctorRecord?.consultationLocation || '未设置' }}</p>
            </div>
          </div>
        </div>
      </div>

      <!-- Stats Dashboard -->
      <div class="grid grid-cols-2 lg:grid-cols-4 gap-4">
        <!-- Total -->
        <div class="bg-white rounded-2xl border border-slate-200/60 p-4 shadow-sm hover:shadow-md transition-shadow">
          <div class="flex items-center justify-between">
            <div>
              <p class="text-xs text-slate-400 font-medium mb-1">总预约</p>
              <p class="text-2xl font-bold text-slate-800">{{ stats.total }}</p>
            </div>
            <div class="h-10 w-10 rounded-xl bg-blue-50 flex items-center justify-center">
              <Users class="w-5 h-5 text-blue-500" />
            </div>
          </div>
        </div>

        <!-- Pending -->
        <div class="bg-white rounded-2xl border border-amber-200/60 p-4 shadow-sm hover:shadow-md transition-shadow cursor-pointer"
          @click="activeTab = 'PENDING'">
          <div class="flex items-center justify-between">
            <div>
              <p class="text-xs text-slate-400 font-medium mb-1">待确认</p>
              <p class="text-2xl font-bold text-amber-600">{{ stats.pending }}</p>
            </div>
            <div class="h-10 w-10 rounded-xl bg-amber-50 flex items-center justify-center">
              <AlertCircle class="w-5 h-5 text-amber-500" />
            </div>
          </div>
          <div v-if="stats.pending > 0" class="mt-2 flex items-center gap-1 text-[10px] text-amber-600 font-semibold">
            <ArrowUpRight class="w-3 h-3" /> 需要处理
          </div>
        </div>

        <!-- Confirmed -->
        <div class="bg-white rounded-2xl border border-emerald-200/60 p-4 shadow-sm hover:shadow-md transition-shadow cursor-pointer"
          @click="activeTab = 'CONFIRMED'">
          <div class="flex items-center justify-between">
            <div>
              <p class="text-xs text-slate-400 font-medium mb-1">已确认</p>
              <p class="text-2xl font-bold text-emerald-600">{{ stats.confirmed }}</p>
            </div>
            <div class="h-10 w-10 rounded-xl bg-emerald-50 flex items-center justify-center">
              <UserCheck class="w-5 h-5 text-emerald-500" />
            </div>
          </div>
        </div>

        <!-- Today -->
        <div class="bg-white rounded-2xl border border-indigo-200/60 p-4 shadow-sm hover:shadow-md transition-shadow">
          <div class="flex items-center justify-between">
            <div>
              <p class="text-xs text-slate-400 font-medium mb-1">今日就诊</p>
              <p class="text-2xl font-bold text-indigo-600">{{ stats.todayCount }}</p>
            </div>
            <div class="h-10 w-10 rounded-xl bg-indigo-50 flex items-center justify-center">
              <Calendar class="w-5 h-5 text-indigo-500" />
            </div>
          </div>
        </div>
      </div>

      <!-- Filter Tabs -->
      <div class="bg-white rounded-2xl border border-slate-200/60 shadow-sm overflow-hidden">
        <!-- Tab Bar -->
        <div class="border-b border-slate-100 px-4 sm:px-6 flex items-center gap-1 overflow-x-auto">
          <button v-for="tab in [
            { key: 'ALL', label: '全部', icon: Users },
            { key: 'PENDING', label: '待确认', icon: AlertCircle },
            { key: 'CONFIRMED', label: '已确认', icon: CheckCircle },
            { key: 'CANCELLED', label: '已取消', icon: XCircle }
          ]" :key="tab.key"
            @click="activeTab = tab.key"
            :class="['flex items-center gap-1.5 px-4 py-3 text-xs font-bold whitespace-nowrap border-b-2 transition-all',
              activeTab === tab.key
                ? tab.key === 'PENDING' ? 'border-amber-500 text-amber-700' :
                  tab.key === 'CONFIRMED' ? 'border-emerald-500 text-emerald-700' :
                  tab.key === 'CANCELLED' ? 'border-slate-500 text-slate-700' :
                  'border-blue-500 text-blue-700'
                : 'border-transparent text-slate-400 hover:text-slate-600 hover:bg-slate-50']">
            <component :is="tab.icon" class="w-3.5 h-3.5" />
            {{ tab.label }}
            <span :class="['ml-1 px-1.5 py-0.5 rounded-full text-[9px] font-bold',
              activeTab === tab.key ? 'bg-current/10' : 'bg-slate-100 text-slate-500']">
              {{ tab.key === 'ALL' ? allAppts.length : allAppts.filter(a => a.status === tab.key).length }}
            </span>
          </button>
        </div>

        <!-- Appointment List -->
        <div class="p-4 sm:p-6">
          <!-- Empty State -->
          <div v-if="filteredAppts.length === 0" class="py-20 text-center">
            <div class="h-16 w-16 mx-auto mb-4 rounded-2xl bg-slate-100 flex items-center justify-center">
              <Calendar class="w-8 h-8 text-slate-300" />
            </div>
            <h4 class="text-slate-400 font-semibold mb-1">暂无{{ activeTab === 'ALL' ? '' : statusLabel(activeTab) }}预约</h4>
            <p class="text-xs text-slate-300">当有新的预约时会显示在这里</p>
          </div>

          <!-- Grouped by Time -->
          <div v-else class="space-y-4">
            <div v-for="groupKey in ['today', 'upcoming', 'past']" :key="groupKey">
              <template v-if="hasGroupContent(groupKey)">
                <!-- Group Header -->
                <button @click="toggleGroup(groupKey)"
                  class="w-full flex items-center justify-between px-4 py-2.5 rounded-xl bg-slate-50/80 hover:bg-slate-100 transition-colors mb-3">
                  <div class="flex items-center gap-3">
                    <span class="text-sm font-bold text-slate-700">{{ groupLabel(groupKey) }}</span>
                    <span class="bg-white px-2 py-0.5 rounded-full text-[10px] font-bold text-slate-500 border border-slate-200">
                      {{ groupCount(groupKey) }}
                    </span>
                  </div>
                  <component :is="expandedGroups[groupKey] ? ChevronUp : ChevronDown" class="w-4 h-4 text-slate-400" />
                </button>

                <!-- Group Cards -->
                <div v-show="expandedGroups[groupKey]" class="space-y-3">
                  <div v-for="appt in groupedAppts[groupKey]" :key="appt.id"
                    :class="['rounded-2xl border p-4 transition-all hover:shadow-md',
                      appt.status === 'PENDING' ? 'bg-amber-50/40 border-amber-200/60 hover:border-amber-300' :
                      appt.status === 'CONFIRMED' ? 'bg-emerald-50/30 border-emerald-200/60' :
                      'bg-slate-50/50 border-slate-200/60 opacity-60']">

                    <div class="flex flex-col sm:flex-row sm:items-center gap-4">
                      <!-- Left: Time Block -->
                      <div class="flex sm:flex-col items-center sm:items-center gap-2 sm:gap-0 sm:min-w-[100px] sm:text-center">
                        <div class="text-xl font-bold text-slate-800 font-mono tracking-tight">
                          {{ formatTime(appt.startTime) }}
                        </div>
                        <div class="text-[10px] text-slate-400 font-medium">
                          ~ {{ formatTime(appt.endTime) }}
                        </div>
                        <div class="text-[10px] font-semibold px-2 py-0.5 rounded-full sm:mt-1"
                          :class="appt.appointmentDate === new Date().toISOString().split('T')[0]
                            ? 'bg-blue-100 text-blue-700' : 'bg-slate-100 text-slate-500'">
                          {{ formatDate(appt.appointmentDate) }}
                        </div>
                      </div>

                      <!-- Divider -->
                      <div class="hidden sm:block w-px h-14 bg-slate-200"></div>

                      <!-- Middle: Patient & Info -->
                      <div class="flex-1 min-w-0">
                        <div class="flex items-center gap-2 flex-wrap">
                          <div class="h-7 w-7 rounded-full bg-gradient-to-br from-slate-500 to-slate-700 flex items-center justify-center text-white text-[10px] font-bold shrink-0">
                            {{ getUserName(appt.userId)[0] }}
                          </div>
                          <span class="text-sm font-bold text-slate-800">{{ getUserName(appt.userId) }}</span>
                          <span class="bg-white/80 px-2 py-0.5 rounded-md text-[10px] font-semibold text-slate-500 border border-slate-200">
                            {{ getDeptName(appt.departmentId) }}
                          </span>
                        </div>
                        <div class="mt-1.5 flex items-center gap-3 text-[10px] text-slate-400 flex-wrap">
                          <span v-if="getUserInfo(appt.userId).phone" class="flex items-center gap-1">
                            📱 {{ getUserInfo(appt.userId).phone }}
                          </span>
                          <span>🆔 APP-{{ String(appt.id).slice(-6) }}</span>
                        </div>
                        <p v-if="appt.remark" class="mt-1.5 text-[10px] text-slate-500 italic bg-white/60 rounded-lg px-2.5 py-1 border border-slate-100">
                          💬 {{ appt.remark }}
                        </p>
                      </div>

                      <!-- Right: Status & Actions -->
                      <div class="flex sm:flex-col items-center sm:items-end gap-2 shrink-0">
                        <!-- Status Badge -->
                        <span :class="['px-3 py-1.5 rounded-xl text-[10px] font-bold tracking-wide',
                          appt.status === 'CONFIRMED' ? 'bg-emerald-500/10 text-emerald-700 border border-emerald-300/50' :
                          appt.status === 'CANCELLED' ? 'bg-slate-200 text-slate-500' :
                          'bg-amber-500/10 text-amber-700 border border-amber-300/50 animate-pulse']">
                          {{ statusLabel(appt.status) }}
                        </span>

                        <!-- Action Buttons (only for PENDING) -->
                        <div v-if="appt.status === 'PENDING'" class="flex gap-1.5">
                          <button @click="confirmAppt(appt)"
                            class="px-3 py-1.5 bg-emerald-500 hover:bg-emerald-600 text-white rounded-lg text-[10px] font-bold flex items-center gap-1 shadow-sm shadow-emerald-500/20 transition-all hover:scale-105 active:scale-95">
                            <CheckCircle class="w-3 h-3" /> 确认
                          </button>
                          <button v-if="showRejectConfirm !== appt.id" @click="showRejectConfirm = appt.id"
                            class="px-3 py-1.5 bg-white hover:bg-rose-50 text-rose-500 border border-rose-200 rounded-lg text-[10px] font-bold flex items-center gap-1 transition-all">
                            <XCircle class="w-3 h-3" /> 拒绝
                          </button>
                          <template v-else>
                            <button @click="rejectAppt(appt)"
                              class="px-3 py-1.5 bg-rose-500 hover:bg-rose-600 text-white rounded-lg text-[10px] font-bold transition-all">
                              确认拒绝
                            </button>
                            <button @click="showRejectConfirm = null"
                              class="px-3 py-1.5 bg-slate-100 hover:bg-slate-200 text-slate-500 rounded-lg text-[10px] font-bold transition-all">
                              取消
                            </button>
                          </template>
                        </div>

                        <!-- Creation time -->
                        <span class="text-[9px] text-slate-300">
                          {{ new Date(appt.createTime).toLocaleDateString('zh-CN') }}
                        </span>
                      </div>
                    </div>
                  </div>
                </div>
              </template>
            </div>
          </div>
        </div>
      </div>
    </main>
  </div>
</template>

<style scoped>
.fade-enter-active, .fade-leave-active { transition: opacity 0.2s ease; }
.fade-enter-from, .fade-leave-to { opacity: 0; }
</style>
