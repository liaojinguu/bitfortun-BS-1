#概述
OTP 模块为 Web API 提供基于 TOTP 的双因素认证（2FA）和签名会话令牌功能。

主要特性：

TOTP（RFC 4226/6238）：6位数字，30秒步长，HMAC-SHA1 算法

重放攻击防护：±1 时间窗口，配合 3 位掩码

会话令牌：紧凑的 HMAC-SHA256 签名令牌

密钥分离：TOTP 密钥与会话签名密钥相互独立

OTP 尝试速率限制：防止暴力破解

本文档涵盖数据格式、流程、HTTP API、持久化存储和安全考虑。

组件与文件
otp.cpp/.h：TOTP 核心、Base32 编解码、HMAC、会话令牌生成/验证、QR 码生成、设置

http_utils.*：HTTP 辅助函数、JSON 处理、速率限制、请求头解析、OTP/会话验证

handler_otp.*： OTP 的 HTTP 端点：注册、状态查询、会话令牌颁发

qrcodegen.*： 为 otpauth URI 生成 QR 码

nvs_config. / Config::：** 非易失性存储（NVS）持久化

加密与数据格式
Base32 编码
字母表：RFC 4648 大写字母，不使用填充字符 "="

编码器：输出最小长度字符数

解码器：跳过非字母表字符（对处理外部输入有用）

从 NVS 加载时，严格验证解码后的长度

长度计算：

字节 → Base32：ceil(字节数 × 8 / 5)

12 字节 → 20 字符

32 字节 → 52 字符

HMAC 算法
TOTP/HOTP：HMAC-SHA1（Google Authenticator/Authy 默认）

会话令牌：HMAC-SHA256

TOTP 参数
数字位数：6

时间周期：30 秒

时间步长：step = now() / 30

重放攻击防护
接受 step-1、step、step+1 三个时间步

3 位掩码（索引 0..2）防止在 ±1 窗口内重复使用已接受的验证码

成功验证后持久化保存基础时间步和掩码状态

会话令牌
设计目标
通过颁发服务端验证的签名会话令牌，减少重复的 TOTP 提示。
密钥分离原则：会话密钥不等于 TOTP 密钥。

令牌结构
text
token = base32(payload) + "." + base32(hmac)

payload (12 字节, 打包存储, 平台字节序):
    uint32_t iat;  // 签发时间 (UNIX 秒)
    uint32_t exp;  // 过期时间 (UNIX 秒)
    uint32_t bid;  // 启动 ID (见下文)

hmac (32 字节):
    HMAC_SHA256(key = m_sessKey, msg = base32(payload))
Base32 长度：

base32(payload) = 20 字符

base32(hmac) = 52 字符

总计 = 73 字符（包含点号）

启动 ID（bid）
随机的 32 位值

当前设计中，bid 持久化存储（NVS），重启后保持不变；但启动新注册时会重新生成（新的会话密钥 + 新的 bid）

效果：重启不会使会话失效；启动新注册会使所有之前的会话失效

流程
注册（初始设置 / 密钥轮换）
会话设置：createSessionKey() 生成

新的 m_sessKey（32 字节随机数，Base32 持久化）

新的 m_bootId（随机数，持久化）

TOTP 密钥：20 字节随机数 → Base32（紧凑格式）

otpauth URI：otpauth://totp/<issuer>:<label>?secret=<B32>&issuer=<issuer>

QR 码：将 URI 编码为 QR 码（版本 ≤ 10，中等纠错级别），在设备 UI 上显示

启用：成功验证 TOTP 后，enabled = true 与密钥一起存储

关键点：启动新注册会隐式轮换 m_sessKey 和 m_bootId → 所有现有会话失效。

登录 / 访问控制
优先：使用请求头 X-OTP-Session 中的会话令牌

备选：使用请求头 X-TOTP 中的 TOTP

OTP 启用时的验证顺序：

如果 force == false（非强制模式）：

如果存在 X-OTP-Session → 验证会话令牌（HMAC/过期时间/bid）

否则（或令牌无效）→ 验证 X-TOTP（±1 窗口，重放掩码）

如果 force == true（关键操作）：

只接受 TOTP，忽略会话令牌

验证成功后，更新重放状态并持久化保存。

HTTP 请求头与端点
请求头
X-TOTP：6 位数字验证码（允许空格）

X-OTP-Session：会话令牌（73 字符，见格式说明）

端点（函数名对应处理函数）
POST_create_otp
目的：启动注册，生成 QR 码并在 UI 上显示

检查：网络访问允许；OTP 不能已启用

响应：成功返回 200；错误返回 405/500

PATCH_update_otp
目的：完成注册，设置启用状态

检查：网络访问允许；需要 TOTP（force = true）

JSON 请求体：{ "enabled": true | false }

效果：注册完成，隐藏 QR 码，存储标志位和密钥

GET_otp_status
目的：查询 OTP 状态

响应（JSON）：{ "enabled": <bool> }

POST_create_otp_session
目的：颁发会话令牌（默认 TTL：24 小时）

检查：网络访问允许；OTP 已启用；需要 TOTP

响应（JSON）：

json
{
  "token": "<X-OTP-Session>",
  "ttlMs": 86400000,
  "expiresAt": <毫秒时间戳>
}
注意：实际 URL 路径取决于 HTTP 路由配置；此处名称对应 C 处理函数。

持久化存储（NVS / Config）
存储的键值（通过 Config::* 符号访问）：

OTPEnabled（布尔值）

OTPSecret（Base32 编码）

OTPSessionKey（32 字节的 Base32 编码；期望恰好 52 字符 → 解码后 32 字节）

OTTBootId（32 位无符号整数）

OTPReplayState

base_step（64 位有符号整数）

used_mask（8 位无符号整数，仅使用 0..2 位）

加载时验证：

OTPSessionKey：Base32 解码后必须恰好为 32 字节；否则丢弃并标记密钥不存在

OTPSecret：按需解码；失败则 OTP 不可用，直到重新注册

速率限制（无效登录尝试）
参数：

RL_FAIL_LIMIT = 5 次尝试

RL_WINDOW_SEC = 60 秒窗口

RL_BLOCK_SEC = 300 秒封锁时间

机制：

维护最近的失败时间戳（毫秒精度）

如果 60 秒内发生 5 次失败：封锁后续尝试 5 分钟（后续失败不会延长封锁时间）

封锁过期后，清除封锁状态并重置历史记录

注意：

封锁状态不持久化（仅进程本地）

封锁期间返回最少错误信息

错误码与响应
401 Unauthorized（未授权）：

缺少/无效的 OTP 或会话令牌

因速率限制处于活跃封锁状态

405 Method Not Allowed（方法不允许）：

OTP 已启用时尝试启动注册

OTP 禁用时尝试颁发会话

400 Bad Request（错误请求）：

JSON 格式无效或缺少必要字段

500 Internal Server Error（服务器内部错误）：

资源/NVS 错误、QR 码/序列化失败、持久化数据无效

响应体：

状态/会话端点返回 JSON

其他错误返回简短文本消息

安全考虑与设计决策
密钥分离
TOTP 密钥 ≠ 会话密钥。支持针对性轮换，限制安全影响范围。

通过 bid 绑定令牌
bid 与注册周期绑定（重启后稳定，注册时轮换）。
效果：重启不会使会话失效；注册轮换会使会话失效。

常量时间 HMAC 比较
会话 HMAC 验证使用常量时间比较（mbedtls_ssl_safer_memcmp）防止时序侧信道攻击。

TOTP 重放保护
±1 窗口配合 3 位掩码，防止在时间窗口内重复使用已接受的验证码。

NVS 的严格 Base32 验证
从 NVS 加载的会话密钥必须解码为恰好 32 字节；否则被拒绝。

速率限制
服务端限流防止暴力破解；毫秒级精确计时。

关键路径的认证顺序
在 PATCH_update_otp 中，UI/状态变更仅在成功 TOTP 验证后发生。

简洁参考（伪代码）
TOTP 验证（简化版）
text
解析 user_code（6 位数字；忽略空格）
解码 key = Base32(secret)

(base_step, used_mask) = load_replay_state()
step = now()/30

if base_step == 0 or step > base_step+1:
    base_step = step
    used_mask = 0
else if step < base_step-1:
    return INVALID  // 时间太旧

for off in {-1,0,+1}:
    s = step + off
    idx = s - (base_step - 1)   // 0..2
    if idx not in [0..2]: continue
    if used_mask & (1<<idx): continue
    if hotp_sha1(key, s) == user_code:
        used_mask |= (1<<idx)
        save_replay_state(base_step, used_mask & 0x07)
        return OK

return INVALID
会话令牌验证（简化版）
text
按 '.' 分割 token
payload_b32 = 左半部分
sig_b32     = 右半部分

p    = base32_decode(payload_b32)  // 12 字节
sref = base32_decode(sig_b32)      // 32 字节

mac = HMAC_SHA256(m_sessKey, payload_b32)
if !ct_equal(mac, sref): return INVALID

pl = parse(payload)
ts = now()

if pl.exp <= ts: return INVALID
if pl.iat > ts + 300: return INVALID  // 最大偏差 5 分钟
if pl.bid != m_bootId: return INVALID

return OK
互操作性
认证器应用：与 Google Authenticator、Authy、1Password 等兼容（SHA1/6位/30秒默认值；标准 otpauth URI）

QR 码：版本 ≤ 10，中等纠错级别（可在 170 像素小屏幕上读取）

维护与轮换
会话密钥轮换：启动新注册生成新的 m_sessKey → 所有会话失效（如有需要可添加管理员轮换端点）

TOTP 密钥轮换：重新运行注册以配置新密钥（用户必须在应用中重新扫描 QR 码）

时间源：TOTP 需要正确的系统时间（is_time_synced()）

限制与已知问题
Payload 字节序：SessionPayload 使用平台字节序（内部使用）。外部解析器应依赖 HMAC 保护的 Base32 字符串，而非原始二进制布局。

Base32 解码器宽容性：跳过非字母表字符。对于 NVS 加载，严格检查长度。

速率限制：进程本地；无全局/持久计数器。

示例：otpauth URI
text
otpauth://totp/<issuer>:<label>?secret=<BASE32>&issuer=<issuer>
issuer 和 label 经过 URL 编码

默认参数（SHA1/6位/30秒）省略以保持 URI 简洁

变更日志（节选）
2025-10-19：

会话令牌使用常量时间 HMAC 比较

速率限制统一使用毫秒精度时间戳

强化 PATCH_update_otp 中的认证顺序

NVS 加载时严格验证 OTPSessionKey（52 个 Base32 字符 → 32 字节）

创建/更新本文档
