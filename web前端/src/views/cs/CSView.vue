<script setup>
import { ref, onMounted, onUnmounted, nextTick } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../../stores/user'
import { MessageSquare, Users, Send, CheckCircle, ChevronRight, RefreshCw } from 'lucide-vue-next'
import { request as api } from '../../api'

const store = useUserStore()
const router = useRouter()
const user = store.user

const csIsOnline = ref(true)
const csPendingQueue = ref([])
const csActiveChats = ref([])
const selectedActiveChat = ref(null)
const csChatMessagesList = ref([])
const csChatInputText = ref('')
const actionMessage = ref(null)
const isGlobalLoading = ref(false)
const showEndConfirm = ref(false)   // 结束会话确认弹窗
const endingConvId = ref(null)      // 待结束的会话ID
let pollTimer = null

// ── 快捷消息 ──
const quickMessages = [
  '您好，请问有什么可以帮助您的？',
  '请您稍等，我来帮您查询一下。',
  '感谢您的耐心等待。',
  '请问还有其他需要帮助的吗？',
  '如果您没有其他问题，我这边就结束会话了。',
  '感谢您的咨询，祝您生活愉快！'
]
function insertQuickMsg(text) {
  csChatInputText.value = text
}
function sendQuickMsg(text) {
  csChatInputText.value = text
  sendMsg()
  // 发送后焦点回到输入框，避免回车重复触发快捷按钮
  nextTick(() => {
    const input = document.getElementById('cs-input')
    if (input) input.focus()
  })
}

function showToast(text, type = 'success') { actionMessage.value = { text, type }; setTimeout(() => { if (actionMessage.value?.text === text) actionMessage.value = null }, 4000) }

async function fetchData() {
  csPendingQueue.value = await api('/api/cs/pending-queue')
  const res = await api('/api/conversation/list?status=ACTIVE&type=MANUAL')
  csActiveChats.value = (res.records || []).filter(c => c.csStaffId === user.id)

  // 检查当前选中的会话是否还在活跃列表中（用户可能已主动结束）
  if (selectedActiveChat.value) {
    const stillActive = csActiveChats.value.find(c => c.id === selectedActiveChat.value.id)
    if (stillActive) {
      // 会话仍活跃 → 同步最新消息
      await syncMessages()
    } else {
      // 会话已被结束 → 清空右侧聊天面板
      selectedActiveChat.value = null
      csChatMessagesList.value = []
    }
  }

  // 自动选中第一个活跃会话
  if (csActiveChats.value.length > 0 && !selectedActiveChat.value) {
    await selectChat(csActiveChats.value[0])
  }
}
async function toggleStatus() {
  const ep = csIsOnline.value ? '/api/cs/staff-online' : '/api/cs/staff-offline'
  await api(`${ep}?csStaffId=${user.id}`, { method: 'POST' })
  showToast(csIsOnline.value ? '已上线' : '已下线', 'info')
  await fetchData()
}
async function selectChat(chat) { selectedActiveChat.value = chat; await syncMessages() }
async function syncMessages() {
  if (!selectedActiveChat.value) return
  csChatMessagesList.value = await api(`/api/message/conversation/${selectedActiveChat.value.id}`)
  await nextTick(); const box = document.getElementById('cs-msg-box'); if (box) box.scrollTop = box.scrollHeight
}
async function sendMsg() {
  const c = csChatInputText.value.trim()
  if (!c || !selectedActiveChat.value) return
  csChatInputText.value = ''
  try {
    await api('/api/message', { method: 'POST', body: JSON.stringify({ conversationId: selectedActiveChat.value.id, senderId: user.id, senderType: 'CS_STAFF', content: c }) })
    await syncMessages()
  } catch (e) {
    showToast('发送失败：' + e.message, 'error')
  }
}
async function acceptQueue(convId) {
  isGlobalLoading.value = true
  try {
    const result = await api(`/api/cs/accept?csStaffId=${user.id}&conversationId=${convId}`, { method: 'POST' })
    showToast(result.message || '已接管会话！', 'success')
    // 获取完整会话信息并选中
    const convDetail = await api(`/api/conversation/${convId}`)
    selectedActiveChat.value = convDetail
    await fetchData()
  } catch (e) {
    showToast('接管失败：' + e.message, 'error')
  } finally { isGlobalLoading.value = false }
}
function promptEndSession(convId) {
  endingConvId.value = convId
  showEndConfirm.value = true
}

async function confirmEndSession() {
  const convId = endingConvId.value
  showEndConfirm.value = false
  endingConvId.value = null
  if (!convId) return

  isGlobalLoading.value = true
  try {
    await api(`/api/cs/close-session?csStaffId=${user.id}&conversationId=${convId}`, { method: 'POST' })
    showToast('会话已关闭', 'success')
    // 立即从本地列表中移除
    csActiveChats.value = csActiveChats.value.filter(c => c.id !== convId)
    if (selectedActiveChat.value?.id === convId) {
      selectedActiveChat.value = null
      csChatMessagesList.value = []
    }
    await fetchData()
  } catch (e) {
    showToast('关闭失败：' + e.message, 'error')
  } finally { isGlobalLoading.value = false }
}
function logout() {
  clearInterval(pollTimer)
  pollTimer = null
  // 通知后端下线（fire-and-forget）
  api(`/api/cs/staff-offline?csStaffId=${user.id}`, { method: 'POST' }).catch(() => {})
  // 强制清除所有登录状态
  try { store.logout() } catch {}
  try { sessionStorage.clear() } catch {}
  // 多级跳转保底
  try { window.location.replace('/login') } catch { window.location.href = '/login' }
}

onMounted(async () => {
  try {
    await api(`/api/cs/staff-online?csStaffId=${user.id}`, { method: 'POST' })
    await fetchData()
  } catch {
    // 后端未启动时静默，页面显示空状态而非白屏
    csPendingQueue.value = []
    csActiveChats.value = []
  }
  pollTimer = setInterval(() => { fetchData().catch(() => {}) }, 5000)
})
onUnmounted(() => clearInterval(pollTimer))
</script>

<template>
  <div class="min-h-screen bg-slate-50 text-slate-800">
    <Transition name="fade"><div v-if="actionMessage" :class="['fixed top-5 left-1/2 -translate-x-1/2 z-50 p-4 rounded-xl shadow-xl flex items-center gap-3 border text-sm font-medium', actionMessage.type==='success'?'bg-emerald-50 border-emerald-200 text-emerald-800':'bg-blue-50 border-blue-200 text-blue-800']"><CheckCircle class="w-4 h-4 text-emerald-600" />{{ actionMessage.text }}</div></Transition>

    <header class="bg-white border-b border-slate-200 sticky top-0 z-40 shadow-xs">
      <div class="max-w-7xl mx-auto px-4 py-3 sm:px-6 lg:px-8 flex items-center justify-between">
        <div class="flex items-center gap-3"><div class="bg-emerald-600 text-white p-2 rounded-xl shadow-md"><MessageSquare class="w-5 h-5" /></div><div><h1 class="text-lg font-bold text-slate-900">医慧通 · 客服工作台</h1><p class="text-xs text-slate-400">排队管理 / 即时通讯</p></div></div>
        <div class="flex items-center gap-3">
          <div class="flex items-center gap-2"><span class="text-xs text-slate-500">状态:</span><button @click="csIsOnline=!csIsOnline;toggleStatus()" :class="['px-4 py-1.5 rounded-xl font-bold text-xs shadow-xs flex items-center gap-1.5',csIsOnline?'bg-emerald-600 text-white':'bg-slate-300 text-slate-700']"><span :class="['h-2 w-2 rounded-full bg-white',csIsOnline?'animate-ping':'']"></span>{{ csIsOnline?'在线':'离线' }}</button></div>
          <span class="text-xs text-slate-500">{{ user?.realName }}（客服）</span>
          <button @click="logout" class="text-xs bg-slate-100 hover:bg-slate-200 text-slate-600 px-3 py-1.5 rounded-lg">退出</button>
        </div>
      </div>
    </header>

    <main class="max-w-7xl mx-auto px-4 py-6 sm:px-6 lg:px-8">
      <div v-if="isGlobalLoading" class="fixed inset-0 bg-white/60 z-50 flex items-center justify-center"><div class="bg-slate-900 text-white p-6 rounded-2xl flex items-center gap-4 shadow-2xl"><RefreshCw class="w-6 h-6 animate-spin text-blue-500" /><span>处理中...</span></div></div>

      <div class="grid grid-cols-1 lg:grid-cols-12 gap-6">
        <!-- Left: Queue + Active -->
        <section class="col-span-12 lg:col-span-4 space-y-6">
          <div class="bg-white border border-slate-200 rounded-2xl p-4 shadow-sm space-y-3">
            <div class="flex items-center justify-between border-b border-slate-100 pb-2"><h3 class="text-xs font-extrabold text-slate-800 uppercase tracking-wider flex items-center gap-1.5"><Users class="w-4 h-4 text-orange-500" />排队队列</h3><span class="bg-orange-100 text-orange-700 font-extrabold text-[10px] px-2 py-0.5 rounded-full font-mono">{{ csPendingQueue.length }} 人</span></div>
            <div v-if="!csPendingQueue.length" class="py-12 text-center text-slate-400 text-xs">暂无等待</div>
            <div class="space-y-2 max-h-[250px] overflow-y-auto"><div v-for="q in csPendingQueue" :key="q.conversationId" class="border border-slate-200 p-3 rounded-xl bg-orange-50/20 flex items-center justify-between text-xs gap-3">
              <div><div class="flex items-center gap-1.5"><span class="bg-orange-600 text-white text-[9px] font-extrabold px-1.5 rounded-full">#{{ q.queuePosition }}</span><strong class="text-slate-800">{{ q.userName }}</strong></div><p class="text-[9px] text-slate-400 mt-1">{{ q.createTime }}</p></div>
              <button @click="acceptQueue(q.conversationId)" class="bg-blue-600 hover:bg-blue-700 text-white text-[10px] font-bold px-3 py-1.5 rounded-lg shrink-0">接入</button>
            </div></div>
          </div>

          <div class="bg-white border border-slate-200 rounded-2xl p-4 shadow-sm space-y-3">
            <div class="flex items-center justify-between border-b border-slate-100 pb-2"><h3 class="text-xs font-extrabold text-slate-800 uppercase tracking-wider flex items-center gap-1.5"><MessageSquare class="w-4 h-4 text-blue-500" />活跃会话</h3><span class="bg-blue-50 text-blue-700 font-extrabold text-[10px] px-2 py-0.5 rounded-full font-mono">{{ csActiveChats.length }} 个</span></div>
            <div class="space-y-2 max-h-[300px] overflow-y-auto"><div v-for="c in csActiveChats" :key="c.id" @click="selectChat(c)" :class="['p-3 rounded-xl border text-xs cursor-pointer flex items-center justify-between',selectedActiveChat?.id===c.id?'bg-blue-50 border-blue-300 shadow-xs':'bg-white border-slate-200 hover:bg-slate-50']"><div><strong class="text-slate-800">用户 #{{ c.userId }}</strong><p class="text-[9px] text-slate-400">会话 {{ c.id }}</p></div><ChevronRight class="w-4 h-4 text-slate-400" /></div></div>
            <div v-if="!csActiveChats.length" class="py-12 text-center text-slate-400 text-xs">暂无活跃会话</div>
          </div>
        </section>

        <!-- Right: Chat Console -->
        <section class="col-span-12 lg:col-span-8 bg-white border border-slate-200 rounded-2xl shadow-sm flex flex-col overflow-hidden" style="height:calc(100vh - 180px);min-height:500px">
          <div v-if="!selectedActiveChat" class="m-auto text-center px-4"><MessageSquare class="w-12 h-12 text-slate-300 mx-auto mb-3" /><h4 class="text-slate-800 font-bold">暂无选定会话</h4><p class="text-xs text-slate-400 max-w-sm mt-1">从左侧选择活跃会话或接入排队用户</p></div>
          <template v-else>
            <div class="bg-slate-900 text-white px-4 py-3 flex justify-between items-center shrink-0"><div><h4 class="text-xs font-bold">服务用户 #{{ selectedActiveChat.userId }}</h4><p class="text-[10px] text-slate-400">会话 {{ selectedActiveChat.id }}</p></div><button @click="promptEndSession(selectedActiveChat.id)" class="bg-rose-600 hover:bg-rose-700 text-white font-bold text-[10px] px-3.5 py-1.5 rounded-lg">结束服务</button></div>
            <div id="cs-msg-box" class="flex-grow p-4 overflow-y-auto space-y-4 bg-slate-50/50"><div v-for="m in csChatMessagesList" :key="m.id" :class="['flex gap-3 max-w-lg',m.senderType==='CS_STAFF'?'ml-auto flex-row-reverse':'']">
              <img :src="m.senderType==='CS_STAFF'?'https://images.unsplash.com/photo-1494790108377-be9c29b29330?auto=format&fit=crop&w=150&q=80':'https://images.unsplash.com/photo-1570295999919-56ceb5ecca61?auto=format&fit=crop&w=150&q=80'" class="w-7 h-7 rounded-full border bg-white shrink-0 mt-1" />
              <div><div :class="['p-3 rounded-2xl text-xs leading-normal shadow-xs',m.senderType==='CS_STAFF'?'bg-slate-900 text-white rounded-tr-none':'bg-white border border-slate-200 text-slate-800 rounded-tl-none']"><p class="whitespace-pre-line">{{ m.content }}</p></div><span class="text-[8px] text-slate-400 block mt-0.5" :class="m.senderType==='CS_STAFF'?'text-right':''">{{ new Date(m.createTime).toLocaleTimeString() }}</span></div>
            </div></div>
            <!-- 快捷消息栏 -->
            <div class="px-4 py-2 border-t border-slate-100 bg-slate-50/80 flex items-center gap-1.5 overflow-x-auto shrink-0">
              <span class="text-[10px] text-slate-400 font-bold shrink-0">快捷消息：</span>
              <button v-for="(qm, i) in quickMessages" :key="i" type="button"
                @click="sendQuickMsg(qm)"
                class="bg-white border border-slate-200 text-slate-600 hover:bg-blue-50 hover:text-blue-700 hover:border-blue-200 px-2.5 py-1 rounded-full text-[10px] transition-all shrink-0 whitespace-nowrap">
                {{ qm.length > 10 ? qm.slice(0, 10) + '...' : qm }}
              </button>
            </div>
            <div class="p-4 bg-white border-t border-slate-200 flex gap-2 shrink-0"><input id="cs-input" v-model="csChatInputText" @keyup.enter="sendMsg" placeholder="输入回复..." class="flex-grow bg-slate-50 border border-slate-200 rounded-xl px-4 py-2 text-xs outline-none focus:bg-white focus:border-blue-500" /><button @click="sendMsg" class="bg-blue-600 hover:bg-blue-700 text-white font-bold px-5 py-2 rounded-xl text-xs flex items-center gap-1"><Send class="w-3.5 h-3.5" />发送</button></div>
          </template>
        </section>
      </div>
    </main>

    <!-- 结束会话确认弹窗 -->
    <Transition name="fade">
      <div v-if="showEndConfirm" class="fixed inset-0 bg-black/40 z-50 flex items-center justify-center" @click.self="showEndConfirm=false;endingConvId=null">
        <div class="bg-white rounded-2xl p-6 shadow-2xl max-w-sm w-full mx-4">
          <h3 class="text-lg font-bold text-slate-900 mb-2">结束会话</h3>
          <p class="text-sm text-slate-500 mb-6">确定结束本次人工服务吗？会话将被关闭，用户将切回 AI 模式。</p>
          <div class="flex gap-3 justify-end">
            <button @click="showEndConfirm=false;endingConvId=null" class="px-4 py-2 text-sm text-slate-500 hover:bg-slate-100 rounded-xl">取消</button>
            <button @click="confirmEndSession" class="px-6 py-2 text-sm font-bold text-white bg-rose-600 hover:bg-rose-700 rounded-xl">确认结束</button>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>
