<template>
	<view class="content">
		<!-- 设备状态展示（仅核心数据） -->
		<view class="device-info">
			<view class="info-item">当前温度 {{ temp }} ℃</view>
			<view class="info-item">当前湿度 {{ humi }} %</view>
			<view class="info-item">运行模式 {{ modeText }}</view>
			<view class="info-item">风速 {{ windyText }}</view>
			<view class="info-item">设置温度 {{ SetTemp }} ℃</view>
		</view>
		
		<!-- 温度控制 -->
		<view class="control-section">
			<text class="control-title">温度调节</text>
			<view class="temp-control">
				<button class="control-btn minus-btn" 
						@click="decreaseTemp" 
						:disabled="SetTemp <= 17 || isSending">
					-
				</button>
				<view class="temp-display">{{ SetTemp }} ℃</view>
				<button class="control-btn plus-btn" 
						@click="increaseTemp" 
						:disabled="SetTemp >= 30 || isSending">
					+
				</button>
			</view>
		</view>
		
		<!-- 模式控制 -->
		<view class="control-section">
			<text class="control-title">模式选择</text>
			<view class="mode-buttons">
				<button class="mode-btn" @click="setMode(0)" :class="{ active: SetMode == 0 }" :disabled="isSending">自动</button>
				<button class="mode-btn" @click="setMode(1)" :class="{ active: SetMode == 1 }" :disabled="isSending">制冷</button>
				<button class="mode-btn" @click="setMode(2)" :class="{ active: SetMode == 2 }" :disabled="isSending">抽湿</button>
				<button class="mode-btn" @click="setMode(3)" :class="{ active: SetMode == 3 }" :disabled="isSending">制热</button>
				<button class="mode-btn" @click="setMode(4)" :class="{ active: SetMode == 4 }" :disabled="isSending">通风</button>
			</view>
		</view>
		
		<!-- 风速控制 -->
		<view class="control-section">
			<text class="control-title">风速选择</text>
			<view class="windy-buttons">
				<button class="windy-btn" @click="setWindy(0)" :class="{ active: SetWindy == 0 }" :disabled="isSending">关闭</button>
				<button class="windy-btn" @click="setWindy(1)" :class="{ active: SetWindy == 1 }" :disabled="isSending">低速</button>
				<button class="windy-btn" @click="setWindy(2)" :class="{ active: SetWindy == 2 }" :disabled="isSending">中速</button>
				<button class="windy-btn" @click="setWindy(3)" :class="{ active: SetWindy == 3 }" :disabled="isSending">高速</button>
				<button class="windy-btn" @click="setWindy(4)" :class="{ active: SetWindy == 4 }" :disabled="isSending">超强</button>
			</view>
		</view>
		
		<!-- 一键发送按钮（无回应等待） -->
		<view class="control-section submit-section">
			<button class="submit-btn" 
					@click="sendAllParams" 
					:disabled="!isModified || isSending">
				{{ isSending ? '发送中...' : '一键发送' }}
			</button>
		</view>
	</view>
</template>

<script>
const { createCommonToken } = require('@/key.js')
export default {
	data() {
		return {
			temp: '11',
			humi: '22',
			SetMode: 1,       // 自动
			SetWindy: 1,      // 低速
			SetTemp: 27,      // 27℃
			token: '',
			timer: null,      // 温湿度定时拉取（已改为1分钟）
			isModified: false, // 是否有修改
			isSending: false   // 是否正在发送（无回应状态）
		}
	},
	computed: {
		modeText() {
			const map = { 0: '自动', 1: '制冷', 2: '抽湿', 3: '制热', 4: '通风' }
			return map[Number(this.SetMode)] || '未知'
		},
		windyText() {
			const map = { 0: '关闭', 1: '低速', 2: '中速', 3: '高速', 4: '超强' }
			return map[Number(this.SetWindy)] || '未知'
		}
	},
	onLoad() {
		// 初始化令牌
		const params = {
			author_key: '',								//填写
			version: '2022-05-01',
			user_id: '',							//个人中心寻找并填写
		}
		this.token = createCommonToken(params)
	},
	onShow() {
		// 拉取当前状态
		this.fetchDevData()
		// 定时更新温湿度（关键修改：从5秒改为60秒/1分钟）
		if (!this.timer) {
			this.timer = setInterval(() => this.fetchDevData(), 60000)
		}
	},
	onHide() {
		// 清除定时器
		if (this.timer) clearInterval(this.timer)
		this.timer = null
	},
	methods: {
		/**
		 * 拉取温湿度和当前设备状态（仅同步显示，不影响发送逻辑）
		 */
		fetchDevData() {
			uni.request({
				url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
				method: 'GET',
				data: { product_id: '', device_name: '' },			//填写
				header: { 'authorization': this.token },
				success: (res) => {
					if (res.data?.data && Array.isArray(res.data.data)) {
						// 更新温湿度
						this.humi = res.data.data[3]?.value || this.humi
						this.temp = res.data.data[4]?.value || this.temp

						// 未发送时同步控制参数（避免本地与实际状态偏差）
						if (!this.isSending) {
							this.SetMode = this.clamp(Number(res.data.data[0]?.value), 0, 4)
							this.SetTemp = this.clamp(Number(res.data.data[1]?.value), 17, 30)
							this.SetWindy = this.clamp(Number(res.data.data[2]?.value), 0, 4)
						}
					}
				}
			})
		},

		/**
		 * 一键发送指令（无回应等待，发送后直接完成）
		 */
		sendAllParams() {
			// 参数未修改时提示
			if (!this.isModified) {
				uni.showModal({
					title: '参数未修改',
					content: '是否仍要发送？',
					success: (res) => res.confirm && this.doSend()
				})
				return
			}
			this.doSend()
		},

		/**
		 * 执行发送（核心：无回应检查 + 新增发送后同步）
		 */
		doSend() {
			// 构造参数
			const params = {
				SetMode: this.clamp(Number(this.SetMode), 0, 4),
				SetTemp: this.clamp(Number(this.SetTemp), 17, 30),
				SetWindy: this.clamp(Number(this.SetWindy), 0, 4)
			}

			// 标记发送中
			this.isSending = true
			uni.request({
				url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
				method: 'POST',
				data: {
					product_id: 'P839UhygiS',
					device_name: 'd2',
					params: params
				},
				header: {
					'authorization': this.token,
					'Content-Type': 'application/json'
				},
				success: (res) => {
					this.isSending = false
					// 只判断请求是否成功，不检查设备是否回应
					if (res.statusCode === 200) {
						uni.showToast({ title: '发送成功', icon: 'success', duration: 1500 })
						this.isModified = false // 重置修改状态
						// 新增：发送成功后主动同步数据，避免等1分钟才更新
						this.fetchDevData()
					} else {
						uni.showToast({ title: '发送失败', icon: 'none', duration: 1500 })
					}
				},
				fail: () => {
					this.isSending = false
					uni.showToast({ title: '网络错误', icon: 'none', duration: 1500 })
				}
			})
		},

		/**
		 * 参数范围控制
		 */
		clamp(value, min, max) {
			const num = Number(value)
			return isNaN(num) ? min : Math.max(min, Math.min(num, max))
		},

		/**
		 * 参数修改方法
		 */
		decreaseTemp() {
			if (this.SetTemp > 17) {
				this.SetTemp--
				this.isModified = true
			}
		},
		increaseTemp() {
			if (this.SetTemp < 30) {
				this.SetTemp++
				this.isModified = true
			}
		},
		setMode(mode) {
			const valid = this.clamp(mode, 0, 4)
			if (this.SetMode !== valid) {
				this.SetMode = valid
				this.isModified = true
			}
		},
		setWindy(speed) {
			const valid = this.clamp(speed, 0, 4)
			if (this.SetWindy !== valid) {
				this.SetWindy = valid
				this.isModified = true
			}
		}
	}
}
</script>

<style>
.content {
	display: flex;
	flex-direction: column;
	padding: 20rpx;
	background-color: #f5f7fa;
	min-height: 100vh;
}

.device-info {
	background-color: #fff;
	border-radius: 16rpx;
	padding: 25rpx;
	margin-bottom: 25rpx;
	box-shadow: 0 2rpx 8rpx rgba(0, 0, 0, 0.05);
}

.info-item {
	font-size: 30rpx;
	color: #333;
	padding: 12rpx 0;
	border-bottom: 1px solid #f5f5f5;
}

.info-item:last-child {
	border-bottom: none;
}

.control-section {
	background-color: #fff;
	border-radius: 16rpx;
	padding: 25rpx;
	margin-bottom: 25rpx;
	box-shadow: 0 2rpx 8rpx rgba(0, 0, 0, 0.05);
}

.control-title {
	font-size: 32rpx;
	font-weight: bold;
	color: #333;
	margin-bottom: 20rpx;
	display: block;
}

.temp-control {
	display: flex;
	align-items: center;
	justify-content: center;
	gap: 25rpx;
}

.control-btn {
	width: 90rpx;
	height: 90rpx;
	border-radius: 50%;
	display: flex;
	align-items: center;
	justify-content: center;
	font-size: 44rpx;
	font-weight: bold;
	border: none;
	color: #fff;
}

.minus-btn {
	background-color: #4285f4;
}

.plus-btn {
	background-color: #ea4335;
}

.temp-display {
	font-size: 44rpx;
	font-weight: bold;
	color: #333;
	width: 140rpx;
	text-align: center;
}

.mode-buttons, .windy-buttons {
	display: flex;
	flex-wrap: wrap;
	gap: 15rpx;
	justify-content: center;
}

.mode-btn, .windy-btn {
	padding: 12rpx 25rpx;
	border-radius: 8rpx;
	font-size: 26rpx;
	border: 1px solid #eee;
	background-color: #f9f9f9;
}

.mode-btn.active, .windy-btn.active {
	background-color: #4285f4;
	color: white;
	border-color: #4285f4;
}

.control-btn:disabled, .mode-btn:disabled, .windy-btn:disabled {
	background-color: #eaeaea !important;
	color: #999 !important;
	border-color: #eaeaea !important;
	opacity: 0.7;
}

.submit-section {
	display: flex;
	justify-content: center;
	padding: 15rpx 20rpx;
}

.submit-btn {
	width: 90%;
	height: 85rpx;
	background-color: #28a745;
	color: white;
	font-size: 30rpx;
	font-weight: bold;
	border-radius: 16rpx;
	border: none;
}

.submit-btn:disabled {
	background-color: #ccc;
	cursor: not-allowed;
}
</style>