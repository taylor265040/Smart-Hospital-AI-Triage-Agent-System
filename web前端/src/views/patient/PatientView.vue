<script setup>
import { ref, computed, onMounted, onUnmounted, nextTick } from 'vue'
import { useUserStore } from '../../stores/user'
import {
  MessageSquare, Send, Calendar, Clock, Bell, Sparkles, X, CheckCircle,
  MapPin, Smile, RefreshCw, Stethoscope, User, ChevronRight, AlertCircle,
  Users, ArrowRight, PlusCircle
} from 'lucide-vue-next'
import { request as api, shouldTriggerHuman } from '../../api'

const store = useUserStore()
const user = store.user

// ── Navigation ──
const activeTab = ref('chat')   // 'chat' | 'booking' | 'appointments'

// ── Announcements ──
const announcements = ref([])

// ── Chat State ──
const chatMessages = ref([])
const chatInput = ref('')
const activeConvId = ref(null)
const queueStatus = ref({ success: false, queuePosition: 0, waitingCount: 0, csStaffName: null, conversationId: null })
const isManualMode = ref(false)
const isSendingMsg = ref(false)
const recommendedDeptPayload = ref(null)
const showRatingModal = ref(false)
const ratingForm = ref({ score: 5, comment: '' })

// ── Booking State ──
const activeDepts = ref([])
const allDocsMap = ref({})
const allDeptsMap = ref({})
const filteredDocs = ref([])
const bookingSlots = ref({})      // docId → slots[]
const expandedDocId = ref(null)   // which doctor card is expanded
const selectedDate = ref(new Date().toISOString().split('T')[0])
const selectedSlot = ref('')      // selected startTime string
const remark = ref('')

// ── Appointments State ──
const myAppts = ref([])
const apptTab = ref('ALL')        // ALL | PENDING | CONFIRMED | CANCELLED

// ── Shared ──
const actionMessage = ref(null)
const isGlobalLoading = ref(false)

let pollTimer = null
let apptPollTimer = null

// ── Computed ──
const activeApptCount = computed(() =>
  myAppts.value.filter(a => a.status === 'PENDING' || a.status === 'CONFIRMED').length
)
const maxAppts = 3
const canBookMore = computed(() => activeApptCount.value < maxAppts)

const filteredAppts = computed(() => {
  if (apptTab.value === 'ALL') return myAppts.value
  return myAppts.value.filter(a => a.status === apptTab.value)
})

// ── Toast ──
function showToast(text, type = 'success') {
  actionMessage.value = { text, type }
  setTimeout(() => { if (actionMessage.value?.text === text) actionMessage.value = null }, 4000)
}

// ── Data Fetching ──
async function fetchBaseData() {
  try {
    announcements.value = await api('/api/announcement/published')
    activeDepts.value = (await api('/api/appointment/departments') || []).sort((a, b) => {
      const na = parseInt(a.floor) || 99, nb = parseInt(b.floor) || 99
      return na - nb || (a.sortOrder || 0) - (b.sortOrder || 0)
    })
    for (const d of activeDepts.value) allDeptsMap.value[d.id] = d
    const allDocs = await api('/api/doctor/list?pageSize=200')
    for (const d of (allDocs.records || [])) allDocsMap.value[d.id] = d
  } catch {}
}

async function fetchAppointments() {
  try {
    const res = await api(`/api/appointment/list?userId=${user.id}&pageSize=50`)
    myAppts.value = res.records || []
  } catch {}
}

// ── Helpers ──
function getDeptName(id) { return allDeptsMap.value[id]?.name || `科室#${id}` }
function getDocName(id) { return allDocsMap.value[id]?.name || `医生#${id}` }
function getDocInfo(id) { return allDocsMap.value[id] || {} }

// ── Booking Actions ──
async function onDeptChange() {
  expandedDocId.value = null
  selectedSlot.value = ''
  bookingSlots.value = {}
  filteredDocs.value = []
  if (!bookingFormDeptId.value) return
  filteredDocs.value = await api(`/api/appointment/doctors?departmentId=${bookingFormDeptId.value}`)
}

const bookingFormDeptId = ref(null)

async function toggleDoctorCard(docId) {
  if (expandedDocId.value === docId) {
    expandedDocId.value = null
    selectedSlot.value = ''
    return
  }
  expandedDocId.value = docId
  selectedSlot.value = ''
  if (!bookingSlots.value[docId]) {
    await loadSlots(docId)
  }
}

async function loadSlots(docId) {
  try {
    bookingSlots.value[docId] = await api(`/api/appointment/slots?doctorId=${docId}&date=${selectedDate.value}`)
  } catch { bookingSlots.value[docId] = [] }
}

async function onDateChange(docId) {
  selectedSlot.value = ''
  await loadSlots(docId)
}

function pickSlot(slotTime) {
  selectedSlot.value = slotTime
}

async function book(docId) {
  if (!bookingFormDeptId.value || !docId || !selectedSlot.value) {
    showToast('请完整选择科室、医生、时段', 'error')
    return
  }
  if (!canBookMore.value) {
    showToast(`您已有 ${maxAppts} 笔有效预约，请先取消后再预约`, 'error')
    return
  }
  isGlobalLoading.value = true
  try {
    await api('/api/appointment/book', {
      method: 'POST',
      body: JSON.stringify({
        userId: user.id,
        doctorId: docId,
        departmentId: bookingFormDeptId.value,
        appointmentDate: selectedDate.value,
        startTime: selectedSlot.value,
        remark: remark.value
      })
    })
    showToast('预约成功！', 'success')
    expandedDocId.value = null
    selectedSlot.value = ''
    remark.value = ''
    bookingSlots.value = {}
    await fetchAppointments()
  } catch (e) { showToast(e.message, 'error') }
  finally { isGlobalLoading.value = false }
}

async function cancelAppt(id) {
  if (!confirm('确定取消该预约吗？')) return
  isGlobalLoading.value = true
  try {
    await api(`/api/appointment/${id}/cancel?userId=${user.id}`, { method: 'PUT' })
    showToast('已取消', 'success')
    await fetchAppointments()
  } catch (e) { showToast(e.message, 'error') }
  finally { isGlobalLoading.value = false }
}

function acceptRecommendation() {
  if (recommendedDeptPayload.value) {
    activeTab.value = 'booking'
    bookingFormDeptId.value = recommendedDeptPayload.value.departmentId
    onDeptChange()
    recommendedDeptPayload.value = null
  }
}

// ── Chat Functions ──
async function startChat() {
  try {
    const list = await api(`/api/conversation/list?userId=${user.id}&status=ACTIVE`)
    if (list.records?.length > 0) {
      const manualConv = list.records.find(c => c.type === 'MANUAL')
      const aiConv = list.records.find(c => c.type === 'AI')
      if (manualConv) {
        let isValidManual = false
        try {
          const qs = await api(`/api/cs/queue-status?userId=${user.id}`)
          if (qs.success && qs.csStaffName && qs.conversationId === manualConv.id) {
            isValidManual = true
            activeConvId.value = manualConv.id
            isManualMode.value = true
            queueStatus.value = qs
            await syncMessages()
          }
        } catch {}
        if (isValidManual) return
        try {
          await api('/api/conversation', { method: 'PUT', body: JSON.stringify({ id: manualConv.id, userId: user.id, type: 'MANUAL', status: 'CLOSED', csStaffId: manualConv.csStaffId || null }) })
        } catch {}
      }
      if (aiConv && aiConv.id !== manualConv?.id) {
        activeConvId.value = aiConv.id
        isManualMode.value = false
        queueStatus.value = { success: false, queuePosition: 0, waitingCount: 0, csStaffName: null, conversationId: null }
        await syncMessages()
        return
      }
    }
    const c = await api('/api/conversation', { method: 'POST', body: JSON.stringify({ userId: user.id, type: 'AI' }) })
    activeConvId.value = c.id
    isManualMode.value = false
    queueStatus.value = { success: false, queuePosition: 0, waitingCount: 0, csStaffName: null, conversationId: null }
    await syncMessages()
  } catch {}
}

async function syncMessages() {
  if (!activeConvId.value) return
  chatMessages.value = await api(`/api/message/conversation/${activeConvId.value}`)
  await nextTick()
  const box = document.getElementById('chat-scroll-box')
  if (box) box.scrollTop = box.scrollHeight
}

async function checkQueue() {
  if (isGlobalLoading.value) return
  try {
    const status = await api(`/api/cs/queue-status?userId=${user.id}`)
    const prevCsName = queueStatus.value.csStaffName
    queueStatus.value = status
    if (status.success && status.conversationId && status.csStaffName) {
      if (!isManualMode.value) {
        if (activeConvId.value !== status.conversationId || !prevCsName) {
          activeConvId.value = status.conversationId
          isManualMode.value = true
          await syncMessages()
          chatMessages.value.push({
            id: Date.now() + '-sys', conversationId: status.conversationId, senderId: 0, senderType: 'AI',
            content: `✨ 人工客服 ${status.csStaffName} 已接入，开始为您服务`, createTime: new Date().toISOString()
          })
          showToast(`人工客服 ${status.csStaffName} 已接入！`, 'success')
        }
      }
      return
    }
    if (status.queuePosition > 0 && !status.csStaffName) { isManualMode.value = false; return }
    if (isManualMode.value && !status.csStaffName && status.queuePosition === 0) {
      try {
        const conv = await api(`/api/conversation/${activeConvId.value}`)
        if (conv.status === 'CLOSED') {
          isManualMode.value = false
          queueStatus.value = { success: false, queuePosition: 0, waitingCount: 0, csStaffName: null, conversationId: null }
          const c = await api('/api/conversation', { method: 'POST', body: JSON.stringify({ userId: user.id, type: 'AI' }) })
          activeConvId.value = c.id
          await syncMessages()
          chatMessages.value.push({
            id: Date.now() + '-sys-end', conversationId: c.id, senderId: 0, senderType: 'AI',
            content: '🤖 人工服务已结束，已切回 AI 智能客服模式。有什么可以帮您的？', createTime: new Date().toISOString()
          })
          showToast('人工服务已结束，已切回 AI 模式', 'info')
        }
      } catch {}
    }
  } catch {}
}

async function sendChatMessage() {
  const msg = chatInput.value.trim()
  if (!msg || !activeConvId.value) return
  chatInput.value = ''
  const containsHumanKeyword = shouldTriggerHuman(msg)
  if (containsHumanKeyword && !isManualMode.value) {
    isSendingMsg.value = true
    // 推送用户消息到本地聊天框（后端负责持久化）
    chatMessages.value.push({ id: Date.now(), conversationId: activeConvId.value, senderId: user.id, senderType: 'USER', content: msg, createTime: new Date().toISOString() })
    await requestHuman()
    isSendingMsg.value = false
    return
  }
  isSendingMsg.value = true
  // 推送用户消息到本地聊天框（后端 AgentOrchestrator 负责持久化到 DB）
  chatMessages.value.push({ id: Date.now(), conversationId: activeConvId.value, senderId: user.id, senderType: 'USER', content: msg, createTime: new Date().toISOString() })
  await nextTick()
  const box = document.getElementById('chat-scroll-box')
  if (box) box.scrollTop = box.scrollHeight
  try {
    if (isManualMode.value) {
      await syncMessages()
    } else {
      const qa = await api('/api/v2/qa/ask', { method: 'POST', body: JSON.stringify({ question: msg, userId: user.id, conversationId: activeConvId.value }) })
      recommendedDeptPayload.value = qa.recommendedDepartment || null
      chatMessages.value.push({ id: Date.now() + '-ai', conversationId: activeConvId.value, senderId: 0, senderType: 'AI', content: qa.answer, createTime: new Date().toISOString() })
      // 不再单独保存 AI 回复 — 后端 AgentOrchestrator 已在管道中持久化
      await nextTick()
      if (box) box.scrollTop = box.scrollHeight
      if (qa.needHumanService) { showToast('AI建议转人工客服，正在为您转接...', 'info'); await requestHuman() }
    }
  } catch (e) { console.error(e); showToast('发送失败：' + e.message, 'error') }
  finally { isSendingMsg.value = false }
}

async function requestHuman() {
  isGlobalLoading.value = true
  // 立即显示正在接通提示
  const connectingId = Date.now() + '-connecting'
  chatMessages.value.push({
    id: connectingId,
    conversationId: activeConvId.value,
    senderId: 0,
    senderType: 'AI',
    content: '🔄 正在为您转接人工客服，请稍候...',
    createTime: new Date().toISOString()
  })
  await nextTick()
  const box = document.getElementById('chat-scroll-box')
  if (box) box.scrollTop = box.scrollHeight
  try {
    const result = await api(`/api/cs/request-human?userId=${user.id}`, { method: 'POST' })
    queueStatus.value = result
    // 移除临时接通提示，替换为实际结果
    chatMessages.value = chatMessages.value.filter(m => m.id !== connectingId)
    if (result.queuePosition > 0) {
      chatMessages.value.push({ id: Date.now() + '-queue', conversationId: activeConvId.value, senderId: 0, senderType: 'AI', content: `⏳ 您已进入人工客服排队队列，当前第 ${result.queuePosition} 位，请耐心等待客服接入...`, createTime: new Date().toISOString() })
      showToast(`已进入排队队列，当前第 ${result.queuePosition} 位`, 'info')
    } else if (result.csStaffName) {
      chatMessages.value.push({ id: Date.now() + '-connected', conversationId: activeConvId.value, senderId: 0, senderType: 'AI', content: `✅ 人工客服 ${result.csStaffName} 已接入，开始为您服务`, createTime: new Date().toISOString() })
      showToast(`人工客服 ${result.csStaffName} 已接入！`, 'success')
    } else if (result.message) { showToast(result.message, 'info') }
  } catch (e) {
    // 移除临时提示
    chatMessages.value = chatMessages.value.filter(m => m.id !== connectingId)
    showToast('转人工失败：' + e.message, 'error')
  } finally { isGlobalLoading.value = false }
}

async function endManualService() {
  isGlobalLoading.value = true
  const oldConvId = activeConvId.value
  try {
    if (isManualMode.value && oldConvId) {
      let fullConv
      try { fullConv = await api(`/api/conversation/${oldConvId}`) } catch {}
      await api('/api/conversation', { method: 'PUT', body: JSON.stringify({ id: oldConvId, userId: user.id, type: 'MANUAL', status: 'CLOSED', csStaffId: fullConv?.csStaffId || queueStatus.value.csStaffId || null }) })
    }
    if (queueStatus.value.queuePosition > 0) { await api(`/api/cs/cancel-queue?userId=${user.id}`, { method: 'POST' }).catch(() => {}) }
  } catch (e) { showToast('关闭会话失败：' + e.message, 'error'); isGlobalLoading.value = false; return }
  try {
    queueStatus.value = { success: false, queuePosition: 0, waitingCount: 0, csStaffName: null, conversationId: null }
    isManualMode.value = false
    const c = await api('/api/conversation', { method: 'POST', body: JSON.stringify({ userId: user.id, type: 'AI' }) })
    activeConvId.value = c.id
    await syncMessages()
    chatMessages.value.push({ id: Date.now() + '-ai-back', conversationId: c.id, senderId: 0, senderType: 'AI', content: '🤖 人工服务已结束，已切回 AI 智能客服模式。有什么可以帮您的？', createTime: new Date().toISOString() })
    showToast('已返回 AI 智能客服模式', 'info')
  } catch (e) { showToast('创建AI会话失败：' + e.message, 'error') }
  finally { isGlobalLoading.value = false }
}

async function submitRating() {
  await api('/api/rating', { method: 'POST', body: JSON.stringify({ conversationId: activeConvId.value, userId: user.id, score: ratingForm.value.score, comment: ratingForm.value.comment }) })
  showToast('感谢评价！', 'success'); showRatingModal.value = false; ratingForm.value = { score: 5, comment: '' }
}

function logout() {
  clearInterval(pollTimer); clearInterval(apptPollTimer)
  pollTimer = null; apptPollTimer = null
  try { store.logout() } catch {}
  try { sessionStorage.clear() } catch {}
  try { window.location.replace('/login') } catch { window.location.href = '/login' }
}

// ── Lifecycle ──
onMounted(async () => {
  await fetchBaseData()
  await fetchAppointments()
  await startChat()
  // Chat polling (every 5s for queue status)
  pollTimer = setInterval(async () => {
    try { await checkQueue() } catch {}
    if (isManualMode.value && activeConvId.value) {
      try { await syncMessages() } catch {}
    }
  }, 5000)
  // Appointment polling (every 10s for real-time status sync)
  apptPollTimer = setInterval(async () => {
    try { await fetchAppointments() } catch {}
  }, 10000)
})

onUnmounted(() => {
  clearInterval(pollTimer)
  clearInterval(apptPollTimer)
})
</script>

<template>
  <div class="min-h-screen bg-slate-50 text-slate-800">
    <!-- Toast -->
    <Transition name="fade">
      <div v-if="actionMessage"
        :class="['fixed top-5 left-1/2 -translate-x-1/2 z-50 p-4 rounded-2xl shadow-2xl flex items-center gap-3 border text-sm font-semibold backdrop-blur-sm',
          actionMessage.type === 'success' ? 'bg-emerald-50/95 border-emerald-200 text-emerald-800' :
          actionMessage.type === 'error' ? 'bg-rose-50/95 border-rose-200 text-rose-800' :
          'bg-blue-50/95 border-blue-200 text-blue-800']">
        <CheckCircle v-if="actionMessage.type === 'success'" class="w-5 h-5 text-emerald-500" />
        <AlertCircle v-else-if="actionMessage.type === 'error'" class="w-5 h-5 text-rose-500" />
        <span v-else class="w-5 h-5 text-blue-500">!</span>
        {{ actionMessage.text }}
      </div>
    </Transition>

    <!-- Header -->
    <header class="bg-white/90 backdrop-blur-md border-b border-slate-200/60 sticky top-0 z-40 shadow-sm">
      <div class="max-w-7xl mx-auto px-4 py-3 sm:px-6 lg:px-8 flex items-center justify-between">
        <div class="flex items-center gap-3">
          <div class="bg-gradient-to-br from-blue-600 to-blue-700 text-white p-2.5 rounded-xl shadow-lg shadow-blue-500/20">
            <MessageSquare class="w-5 h-5" />
          </div>
          <div>
            <h1 class="text-lg font-bold text-slate-900 tracking-tight">医慧通 · 患者端</h1>
            <p class="text-[10px] text-slate-400">智能问诊 / 预约挂号 / 人工咨询</p>
          </div>
        </div>
        <div class="flex items-center gap-3 sm:gap-4">
          <span class="hidden sm:inline text-xs text-slate-500">{{ user?.realName }}（患者）</span>
          <button @click="logout"
            class="text-xs bg-white hover:bg-slate-50 text-slate-500 border border-slate-200 px-3 py-1.5 rounded-lg transition-colors hover:border-slate-300">
            退出
          </button>
        </div>
      </div>
    </header>

    <!-- Navigation Tabs -->
    <div class="bg-white border-b border-slate-200 sticky top-[60px] z-30 shadow-sm">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <nav class="flex gap-1 overflow-x-auto">
          <button @click="activeTab = 'chat'"
            :class="['flex items-center gap-2 px-5 py-3 text-sm font-bold border-b-2 transition-all whitespace-nowrap',
              activeTab === 'chat' ? 'border-blue-500 text-blue-700' : 'border-transparent text-slate-400 hover:text-slate-600 hover:bg-slate-50']">
            <MessageSquare class="w-4 h-4" /> 💬 智能问诊
          </button>
          <button @click="activeTab = 'booking'"
            :class="['flex items-center gap-2 px-5 py-3 text-sm font-bold border-b-2 transition-all whitespace-nowrap',
              activeTab === 'booking' ? 'border-blue-500 text-blue-700' : 'border-transparent text-slate-400 hover:text-slate-600 hover:bg-slate-50']">
            <Calendar class="w-4 h-4" /> 📅 预约挂号
          </button>
          <button @click="activeTab = 'appointments'"
            :class="['flex items-center gap-2 px-5 py-3 text-sm font-bold border-b-2 transition-all whitespace-nowrap',
              activeTab === 'appointments' ? 'border-blue-500 text-blue-700' : 'border-transparent text-slate-400 hover:text-slate-600 hover:bg-slate-50']">
            <Clock class="w-4 h-4" /> 📋 我的预约
            <span v-if="activeApptCount > 0"
              class="bg-blue-500 text-white text-[10px] px-1.5 py-0.5 rounded-full font-bold min-w-[18px] text-center">
              {{ activeApptCount }}
            </span>
          </button>
        </nav>
      </div>
    </div>

    <!-- Main Content -->
    <main class="max-w-7xl mx-auto px-4 py-6 sm:px-6 lg:px-8">
      <!-- Global Loading -->
      <Transition name="fade">
        <div v-if="isGlobalLoading" class="fixed inset-0 bg-white/70 backdrop-blur-sm z-50 flex items-center justify-center">
          <div class="bg-white p-6 rounded-2xl shadow-2xl flex items-center gap-4 border border-slate-100">
            <RefreshCw class="w-6 h-6 animate-spin text-blue-500" />
            <span class="text-sm font-semibold text-slate-600">处理中...</span>
          </div>
        </div>
      </Transition>

      <!-- Announcements -->
      <div v-if="announcements.length" class="mb-6 bg-amber-50 border border-amber-200 p-4 rounded-2xl flex items-start gap-3">
        <Bell class="w-5 h-5 text-amber-600 shrink-0 mt-0.5" />
        <div>
          <h4 class="text-sm font-bold text-amber-900">医院通知</h4>
          <div class="text-xs text-amber-800 mt-1 space-y-1">
            <p v-for="a in announcements" :key="a.id">📢 <strong>【{{ a.title }}】</strong> - {{ a.content }}</p>
          </div>
        </div>
      </div>

      <!-- ═══════════ CHAT TAB ═══════════ -->
      <div v-show="activeTab === 'chat'"
        class="bg-white rounded-2xl border border-slate-200 shadow-sm overflow-hidden flex flex-col"
        style="height:calc(100vh - 240px);min-height:550px">

        <!-- Chat Header -->
        <div class="bg-slate-900 text-white px-5 py-3.5 flex items-center justify-between shrink-0">
          <div class="flex items-center gap-2.5">
            <span :class="['flex h-2.5 w-2.5 rounded-full',
              isManualMode ? 'bg-emerald-500 animate-ping' : 'bg-blue-500 animate-pulse']"></span>
            <div>
              <h3 class="text-sm font-bold">{{ isManualMode && queueStatus.csStaffName ? `人工客服：${queueStatus.csStaffName}` : 'AI 智能客服' }}</h3>
              <p class="text-[10px] text-slate-400">{{ isManualMode ? '一对一即时答疑' : 'AI Agent · 多知识库RAG' }}</p>
            </div>
          </div>
          <div class="flex items-center gap-2">
            <template v-if="queueStatus.queuePosition > 0 && !isManualMode">
              <span class="text-xs bg-amber-600 px-3 py-1.5 rounded-lg text-white font-bold">排队第{{ queueStatus.queuePosition }}位</span>
              <button @click="endManualService" class="text-xs bg-rose-600 hover:bg-rose-700 text-white px-3 py-1.5 rounded-lg">取消排队</button>
            </template>
            <template v-if="isManualMode">
              <span class="text-xs bg-emerald-700 px-3 py-1.5 rounded-lg text-emerald-100">人工已接入</span>
              <button @click="endManualService" class="text-xs bg-rose-600 hover:bg-rose-700 text-white px-3 py-1.5 rounded-lg">结束人工</button>
            </template>
          </div>
        </div>

        <!-- Rating Prompt -->
        <div v-if="chatMessages.length && chatMessages[chatMessages.length-1]?.content?.includes('人工服务已结束')"
          class="bg-blue-50 border-b border-blue-100 px-4 py-3 flex items-center justify-between shrink-0">
          <span class="text-xs text-blue-900 font-medium flex items-center gap-1.5"><Smile class="w-4 h-4 text-blue-600" />会话已关闭</span>
          <button @click="showRatingModal = true" class="bg-blue-600 hover:bg-blue-700 text-white text-xs px-4 py-1.5 rounded-lg">评价服务</button>
        </div>

        <!-- Chat Messages -->
        <div id="chat-scroll-box" class="flex-grow p-5 overflow-y-auto space-y-4 bg-slate-50/50">
          <div v-if="!chatMessages.length" class="flex flex-col items-center justify-center py-20 text-center px-4">
            <div class="bg-blue-100 text-blue-600 p-5 rounded-2xl mb-4 shadow-sm">
              <MessageSquare class="w-10 h-10" />
            </div>
            <h4 class="text-slate-800 font-bold text-lg mb-1">您好！我是医院智能客服</h4>
            <p class="text-sm text-slate-500 max-w-sm">输入症状描述，AI 为您智能解答并推荐科室</p>
          </div>
          <div v-for="msg in chatMessages" :key="msg.id"
            :class="['flex gap-3 max-w-lg', msg.senderType === 'USER' ? 'ml-auto flex-row-reverse' : '']">
            <img :src="msg.senderType === 'USER'
              ? 'https://images.unsplash.com/photo-1570295999919-56ceb5ecca61?auto=format&fit=crop&w=150&q=80'
              : msg.senderType === 'AI'
                ? 'https://images.unsplash.com/photo-1618005182384-a83a8bd57fbe?auto=format&fit=crop&w=150&q=80'
                : 'https://images.unsplash.com/photo-1494790108377-be9c29b29330?auto=format&fit=crop&w=150&q=80'"
              class="w-9 h-9 rounded-full border-2 border-white mt-1 shrink-0 bg-white shadow-sm" />
            <div class="space-y-1 min-w-0">
              <div :class="['p-3.5 rounded-2xl text-sm leading-relaxed shadow-sm',
                msg.senderType === 'USER'
                  ? 'bg-blue-600 text-white rounded-tr-none'
                  : 'bg-white border border-slate-200 text-slate-800 rounded-tl-none']">
                <p class="whitespace-pre-line break-words">{{ msg.content }}</p>
              </div>
              <div class="text-[10px] text-slate-400 flex items-center gap-1.5"
                :class="msg.senderType === 'USER' ? 'justify-end' : ''">
                {{ new Date(msg.createTime).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }) }}
                <span v-if="msg.senderType === 'AI'" class="bg-blue-50 text-blue-600 px-1.5 py-0.5 rounded-full text-[9px] border font-semibold">RAG+AI</span>
                <span v-if="msg.senderType === 'CS_STAFF'" class="bg-emerald-50 text-emerald-600 px-1.5 py-0.5 rounded-full text-[9px] border font-semibold">人工</span>
              </div>
            </div>
          </div>
          <div v-if="isSendingMsg" class="flex gap-2 items-center bg-white p-3 rounded-xl border w-fit shadow-sm">
            <RefreshCw class="w-4 h-4 animate-spin text-blue-600" />
            <span class="text-xs text-slate-500">检索中...</span>
          </div>
        </div>

        <!-- AI Recommendation -->
        <Transition name="slide">
          <div v-if="recommendedDeptPayload"
            class="bg-gradient-to-r from-blue-50 to-indigo-50 border-t border-blue-100 p-4 shrink-0 flex items-center justify-between gap-4">
            <div class="flex items-start gap-3">
              <Sparkles class="w-5 h-5 text-indigo-600 shrink-0 mt-0.5" />
              <div>
                <h5 class="text-sm font-bold text-indigo-900">
                  推荐科室: <span class="bg-indigo-600 text-white py-0.5 px-2.5 rounded-full font-bold ml-1.5">{{ recommendedDeptPayload.departmentName }}</span>
                </h5>
                <p class="text-xs text-indigo-700 mt-1">{{ recommendedDeptPayload.reason }}</p>
              </div>
            </div>
            <div class="flex gap-2 shrink-0">
              <button @click="acceptRecommendation"
                class="bg-indigo-600 hover:bg-indigo-700 text-white text-xs font-bold py-2 px-4 rounded-xl shadow-md flex items-center gap-1.5">
                <ArrowRight class="w-3.5 h-3.5" /> 去预约
              </button>
              <button @click="recommendedDeptPayload = null" class="text-slate-400 hover:text-slate-600 p-1"><X class="w-4 h-4" /></button>
            </div>
          </div>
        </Transition>

        <!-- Chat Input -->
        <div class="p-3 bg-white border-t border-slate-200 flex gap-2 shrink-0">
          <input v-model="chatInput" @keyup.enter="sendChatMessage"
            :placeholder="isManualMode ? '与人工客服对话中...' : '请描述症状或问题，AI 为您解答...'"
            class="flex-grow bg-slate-100 border border-slate-200 focus:bg-white focus:border-blue-500 rounded-xl px-4 py-2.5 text-sm outline-none placeholder:text-slate-400 transition-all" />
          <button @click="sendChatMessage"
            class="bg-blue-600 hover:bg-blue-700 text-white px-5 py-2.5 rounded-xl shadow-md shadow-blue-500/20 transition-all hover:shadow-lg">
            <Send class="w-4 h-4" />
          </button>
        </div>

        <!-- Quick Actions -->
        <div class="px-5 py-3 border-t border-slate-100 bg-slate-50/80 flex items-center gap-2 overflow-x-auto shrink-0">
          <span class="text-[10px] text-slate-400 font-bold shrink-0">常见问题：</span>
          <button @click="chatInput = '发热门诊在哪里？'; sendChatMessage()"
            class="bg-white border border-slate-200 text-slate-600 hover:bg-blue-50 hover:text-blue-700 px-3 py-1.5 rounded-full text-xs transition-all shrink-0">发热门诊</button>
          <button @click="chatInput = '我咳嗽胸闷好几天了，挂什么科？'; sendChatMessage()"
            class="bg-white border border-slate-200 text-slate-600 hover:bg-blue-50 hover:text-blue-700 px-3 py-1.5 rounded-full text-xs transition-all shrink-0">咳嗽看什么科</button>
          <button @click="chatInput = '怎么预约挂号？'; sendChatMessage()"
            class="bg-white border border-slate-200 text-slate-600 hover:bg-blue-50 hover:text-blue-700 px-3 py-1.5 rounded-full text-xs transition-all shrink-0">怎么预约</button>
          <button @click="chatInput = '医保报销需要什么材料？'; sendChatMessage()"
            class="bg-white border border-slate-200 text-slate-600 hover:bg-blue-50 hover:text-blue-700 px-3 py-1.5 rounded-full text-xs transition-all shrink-0">医保报销</button>
        </div>
      </div>

      <!-- ═══════════ BOOKING TAB ═══════════ -->
      <div v-show="activeTab === 'booking'" class="space-y-6">
        <!-- Booking Header -->
        <div class="flex items-center justify-between flex-wrap gap-4">
          <div>
            <h2 class="text-xl font-bold text-slate-900">预约挂号</h2>
            <p class="text-sm text-slate-500 mt-1">选择科室 → 选择医生 → 选择时段 → 完成预约</p>
          </div>
          <div :class="['px-4 py-2 rounded-xl text-sm font-bold flex items-center gap-2',
            canBookMore ? 'bg-emerald-50 text-emerald-700 border border-emerald-200' : 'bg-rose-50 text-rose-700 border border-rose-200']">
            <span :class="['h-2 w-2 rounded-full', canBookMore ? 'bg-emerald-500' : 'bg-rose-500']"></span>
            可预约：{{ maxAppts - activeApptCount }} / {{ maxAppts }} 个名额
          </div>
        </div>

        <!-- Step 1: Department -->
        <div class="bg-white rounded-2xl border border-slate-200 p-5 shadow-sm">
          <div class="flex items-center gap-2 mb-4">
            <span class="h-7 w-7 rounded-full bg-blue-600 text-white text-xs font-bold flex items-center justify-center shrink-0">1</span>
            <h3 class="font-bold text-slate-800">选择科室</h3>
          </div>
          <div class="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 gap-3">
            <button v-for="d in activeDepts" :key="d.id"
              @click="bookingFormDeptId = d.id; onDeptChange()"
              :class="['p-4 rounded-xl border-2 text-left transition-all hover:shadow-md',
                bookingFormDeptId === d.id
                  ? 'border-blue-500 bg-blue-50 shadow-md shadow-blue-100'
                  : 'border-slate-200 bg-white hover:border-blue-300']">
              <div class="h-9 w-9 rounded-lg bg-blue-100 text-blue-600 flex items-center justify-center mb-2">
                <Stethoscope class="w-5 h-5" />
              </div>
              <p class="text-sm font-bold text-slate-800">{{ d.name }}</p>
              <p class="text-[10px] text-slate-400 mt-0.5">{{ d.floor }}</p>
            </button>
          </div>
        </div>

        <!-- Step 2: Doctor Cards -->
        <div v-if="bookingFormDeptId && filteredDocs.length">
          <div class="flex items-center gap-2 mb-4">
            <span class="h-7 w-7 rounded-full bg-blue-600 text-white text-xs font-bold flex items-center justify-center shrink-0">2</span>
            <h3 class="font-bold text-slate-800">选择医生 · {{ getDeptName(bookingFormDeptId) }}</h3>
          </div>
          <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            <div v-for="doc in filteredDocs" :key="doc.id"
              :class="['bg-white rounded-2xl border-2 transition-all overflow-hidden',
                expandedDocId === doc.id ? 'border-blue-400 shadow-lg shadow-blue-100' : 'border-slate-200 hover:border-blue-200 hover:shadow-md']">

              <!-- Card Header -->
              <div class="p-5">
                <div class="flex items-start gap-4">
                  <div class="h-14 w-14 rounded-2xl bg-gradient-to-br from-blue-500 to-indigo-600 flex items-center justify-center text-white text-lg font-bold shadow-md shrink-0">
                    {{ doc.name[0] }}
                  </div>
                  <div class="flex-1 min-w-0">
                    <h4 class="font-bold text-slate-900 text-base">{{ doc.name }}</h4>
                    <p class="text-xs text-blue-600 font-semibold mt-0.5">{{ doc.title }}</p>
                    <p class="text-xs text-slate-500 mt-1.5 line-clamp-2">{{ doc.specialization }}</p>
                  </div>
                </div>
                <p v-if="doc.description" class="text-xs text-slate-400 mt-3 line-clamp-2 leading-relaxed">{{ doc.description }}</p>
                <p v-if="doc.consultationLocation" class="flex items-center gap-1 mt-2 text-[10px] text-slate-500 bg-slate-50 px-2.5 py-1.5 rounded-lg">
                  📍 {{ doc.consultationLocation }}
                </p>

                <!-- Expand/Collapse Button -->
                <button @click="toggleDoctorCard(doc.id)"
                  :class="['mt-4 w-full py-2.5 rounded-xl text-xs font-bold flex items-center justify-center gap-1.5 transition-all',
                    expandedDocId === doc.id
                      ? 'bg-slate-100 text-slate-600 hover:bg-slate-200'
                      : 'bg-blue-600 text-white hover:bg-blue-700 shadow-md shadow-blue-500/20']">
                  <template v-if="expandedDocId === doc.id">
                    <X class="w-3.5 h-3.5" /> 收起
                  </template>
                  <template v-else>
                    <PlusCircle class="w-3.5 h-3.5" /> 选择时段
                  </template>
                </button>
              </div>

              <!-- Expanded Slot Picker -->
              <Transition name="slide">
                <div v-if="expandedDocId === doc.id"
                  class="border-t-2 border-blue-100 bg-gradient-to-b from-blue-50/50 to-white p-5 space-y-4">
                  <!-- Date Picker -->
                  <div>
                    <label class="text-xs font-bold text-slate-600 block mb-1.5">📅 选择日期</label>
                    <input type="date" :value="selectedDate"
                      @change="selectedDate = $event.target.value; onDateChange(doc.id)"
                      :min="new Date().toISOString().split('T')[0]"
                      class="w-full bg-white border border-slate-200 rounded-xl px-3 py-2.5 text-sm font-semibold outline-none focus:border-blue-500 transition-all" />
                  </div>

                  <!-- Slots Grid -->
                  <div>
                    <label class="text-xs font-bold text-slate-600 block mb-2">🕐 选择时段</label>
                    <div v-if="(bookingSlots[doc.id] || []).length" class="grid grid-cols-4 gap-2">
                      <button v-for="s in (bookingSlots[doc.id] || [])" :key="s.startTime"
                        @click="pickSlot(s.startTime)"
                        :disabled="!s.available"
                        :class="['p-2.5 rounded-xl text-center border font-bold transition-all text-xs',
                          !s.available
                            ? 'bg-slate-100 border-slate-200 text-slate-400 cursor-not-allowed line-through'
                            : selectedSlot === s.startTime
                              ? 'bg-blue-600 border-blue-600 text-white shadow-lg shadow-blue-500/20 scale-105'
                              : 'bg-white border-slate-200 text-slate-700 hover:border-blue-300 hover:bg-blue-50']">
                        <div class="text-sm">{{ s.startTime.slice(0, 5) }}</div>
                        <div class="text-[9px] opacity-60">{{ s.available ? s.label : '已约' }}</div>
                      </button>
                    </div>
                    <p v-else class="text-xs text-slate-400 italic py-3 text-center bg-white rounded-xl border border-slate-100">
                      请先选择日期查看可用时段
                    </p>
                  </div>

                  <!-- Remark -->
                  <div v-if="selectedSlot">
                    <label class="text-xs font-bold text-slate-600 block mb-1.5">💬 病情自述（可选）</label>
                    <textarea v-model="remark" placeholder="简述您的症状，方便医生提前了解..."
                      rows="2" class="w-full bg-white border border-slate-200 rounded-xl p-2.5 text-xs outline-none focus:border-blue-500 placeholder:text-slate-300"></textarea>
                  </div>

                  <!-- Book Button -->
                  <button v-if="selectedSlot" @click="book(doc.id)"
                    :disabled="!canBookMore"
                    :class="['w-full font-bold py-3 rounded-xl flex items-center justify-center gap-2 text-sm transition-all',
                      canBookMore
                        ? 'bg-gradient-to-r from-blue-600 to-blue-700 hover:from-blue-700 hover:to-blue-800 text-white shadow-lg shadow-blue-500/20 hover:shadow-xl'
                        : 'bg-slate-200 text-slate-400 cursor-not-allowed']">
                    <CheckCircle class="w-4 h-4" />
                    {{ canBookMore ? `确认预约 · ${doc.name} · ${selectedSlot.slice(0, 5)}` : `已达上限（${maxAppts}笔）` }}
                  </button>
                </div>
              </Transition>
            </div>
          </div>
        </div>

        <!-- Empty state when no dept selected -->
        <div v-if="!bookingFormDeptId"
          class="bg-white rounded-2xl border-2 border-dashed border-slate-300 p-12 text-center">
          <div class="h-16 w-16 mx-auto mb-4 rounded-2xl bg-blue-50 flex items-center justify-center">
            <Stethoscope class="w-8 h-8 text-blue-400" />
          </div>
          <h4 class="text-slate-400 font-semibold mb-1">请先在上方选择一个科室</h4>
          <p class="text-xs text-slate-300">选择科室后将展示该科室下所有在岗医生</p>
        </div>
      </div>

      <!-- ═══════════ APPOINTMENTS TAB ═══════════ -->
      <div v-show="activeTab === 'appointments'" class="space-y-6">
        <!-- Header -->
        <div class="flex items-center justify-between flex-wrap gap-4">
          <div>
            <h2 class="text-xl font-bold text-slate-900">我的预约</h2>
            <p class="text-sm text-slate-500 mt-1 flex items-center gap-1.5">
              <RefreshCw class="w-3 h-3 text-slate-400" /> 每10秒自动刷新状态
            </p>
          </div>
          <div :class="['px-4 py-2 rounded-xl text-sm font-bold flex items-center gap-2',
            canBookMore ? 'bg-emerald-50 text-emerald-700 border border-emerald-200' : 'bg-rose-50 text-rose-700 border border-rose-200']">
            {{ canBookMore ? `还可预约 ${maxAppts - activeApptCount} 个` : '预约名额已满' }}
            · {{ activeApptCount }} / {{ maxAppts }}
          </div>
        </div>

        <!-- Status Tabs -->
        <div class="bg-white rounded-2xl border border-slate-200 shadow-sm overflow-hidden">
          <div class="border-b border-slate-100 px-4 flex gap-1 overflow-x-auto">
            <button v-for="tab in [
              { key: 'ALL', label: '全部', color: 'border-blue-500 text-blue-700' },
              { key: 'PENDING', label: '待确认', color: 'border-amber-500 text-amber-700' },
              { key: 'CONFIRMED', label: '已确认', color: 'border-emerald-500 text-emerald-700' },
              { key: 'CANCELLED', label: '已取消', color: 'border-slate-500 text-slate-700' }
            ]" :key="tab.key"
              @click="apptTab = tab.key"
              :class="['px-4 py-3 text-xs font-bold border-b-2 transition-all whitespace-nowrap',
                apptTab === tab.key ? tab.color : 'border-transparent text-slate-400 hover:text-slate-600']">
              {{ tab.label }}
              <span class="ml-1.5 bg-slate-100 px-1.5 py-0.5 rounded-full text-[10px] text-slate-500">
                {{ tab.key === 'ALL' ? myAppts.length : myAppts.filter(a => a.status === tab.key).length }}
              </span>
            </button>
          </div>

          <!-- Appointment List -->
          <div class="p-4 sm:p-6">
            <div v-if="filteredAppts.length === 0" class="py-16 text-center">
              <div class="h-16 w-16 mx-auto mb-4 rounded-2xl bg-slate-100 flex items-center justify-center">
                <Calendar class="w-8 h-8 text-slate-300" />
              </div>
              <h4 class="text-slate-400 font-semibold mb-1">
                {{ apptTab === 'ALL' ? '暂无预约记录' : '暂无' + { PENDING: '待确认', CONFIRMED: '已确认', CANCELLED: '已取消' }[apptTab] + '的预约' }}
              </h4>
              <p class="text-xs text-slate-300">切换到"预约挂号"标签页进行预约</p>
            </div>

            <div v-else class="space-y-3">
              <div v-for="a in filteredAppts" :key="a.id"
                :class="['border-2 p-4 rounded-2xl transition-all',
                  a.status === 'CONFIRMED' ? 'bg-emerald-50/30 border-emerald-200' :
                  a.status === 'CANCELLED' ? 'bg-slate-50/50 border-slate-200 opacity-60' :
                  'bg-amber-50/30 border-amber-200']">
                <div class="flex flex-col sm:flex-row sm:items-center gap-4">
                  <!-- Time -->
                  <div class="sm:min-w-[90px] sm:text-center">
                    <p class="text-xl font-bold text-slate-800 font-mono">{{ a.startTime?.slice(0, 5) }}</p>
                    <p class="text-[10px] text-slate-400">~ {{ a.endTime?.slice(0, 5) }}</p>
                    <p class="text-[10px] font-semibold text-slate-500 mt-0.5">{{ a.appointmentDate }}</p>
                  </div>

                  <div class="hidden sm:block w-px h-12 bg-slate-200"></div>

                  <!-- Info -->
                  <div class="flex-1 min-w-0">
                    <div class="flex items-center gap-2 flex-wrap">
                      <span class="bg-white/80 px-2 py-0.5 rounded-md text-xs font-bold text-slate-600 border border-slate-200">
                        {{ getDeptName(a.departmentId) }}
                      </span>
                      <span class="text-sm font-bold text-slate-800">👨‍⚕️ {{ getDocName(a.doctorId) }}</span>
                    </div>
                    <p v-if="a.remark" class="mt-1.5 text-[10px] text-slate-400 italic">💬 {{ a.remark }}</p>
                  </div>

                  <!-- Status & Actions -->
                  <div class="flex sm:flex-col items-center sm:items-end gap-2 shrink-0">
                    <span :class="['px-3 py-1.5 rounded-xl text-xs font-bold flex items-center gap-1.5',
                      a.status === 'CONFIRMED' ? 'bg-emerald-100 text-emerald-700 border border-emerald-300/50' :
                      a.status === 'CANCELLED' ? 'bg-slate-200 text-slate-500' :
                      'bg-amber-100 text-amber-700 border border-amber-300/50']">
                      <span :class="['h-1.5 w-1.5 rounded-full',
                        a.status === 'CONFIRMED' ? 'bg-emerald-500' :
                        a.status === 'CANCELLED' ? 'bg-slate-400' :
                        'bg-amber-500 animate-pulse']"></span>
                      {{ a.status === 'CONFIRMED' ? '已确认 ✅' : a.status === 'CANCELLED' ? '已取消' : '待确认 ⏳' }}
                    </span>
                    <button v-if="a.status !== 'CANCELLED'" @click="cancelAppt(a.id)"
                      class="text-rose-500 hover:text-white hover:bg-rose-500 border border-rose-200 hover:border-rose-500 px-3 py-1 rounded-lg text-[10px] font-bold transition-all">
                      取消
                    </button>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </main>

    <!-- Rating Modal -->
    <Transition name="fade">
      <div v-if="showRatingModal" class="fixed inset-0 bg-black/40 z-50 flex items-center justify-center p-4"
        @click.self="showRatingModal = false">
        <div class="bg-white rounded-2xl p-6 shadow-2xl max-w-sm w-full">
          <h3 class="text-lg font-bold mb-4">评价服务</h3>
          <div class="flex justify-center gap-1.5 mb-4 text-3xl">
            <button v-for="s in 5" :key="s" @click="ratingForm.score = s"
              :class="s <= ratingForm.score ? 'text-amber-400' : 'text-slate-300'">★</button>
          </div>
          <textarea v-model="ratingForm.comment" placeholder="请输入评价..." rows="3"
            class="w-full bg-slate-50 border border-slate-200 rounded-xl p-3 text-sm outline-none mb-4"></textarea>
          <div class="flex gap-2 justify-end">
            <button @click="showRatingModal = false" class="px-4 py-2 text-slate-500 text-sm">取消</button>
            <button @click="submitRating" class="bg-blue-600 hover:bg-blue-700 text-white px-6 py-2 rounded-xl text-sm font-bold">提交</button>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<style scoped>
.fade-enter-active, .fade-leave-active { transition: opacity 0.2s ease; }
.fade-enter-from, .fade-leave-to { opacity: 0; }
.slide-enter-active, .slide-leave-active { transition: all 0.25s ease; }
.slide-enter-from, .slide-leave-to { opacity: 0; transform: translateY(-8px); }
.line-clamp-2 { display: -webkit-box; -webkit-line-clamp: 2; -webkit-box-orient: vertical; overflow: hidden; }
</style>
