// Day 25 实验 1：ESP32-S3 连 Wi-Fi，串口打印网络信息
//
// 目标不是"连上就完事"，而是把连上之后的凭据全部打出来 —— 后面 Day 26/27/28
// 所有无线功能都要用到这几项：IP（Web Server 的访问地址）、RSSI（判断信号质量）、
// MAC（路由器里认设备）、信道（1/6/11 之外会跟邻居互相干扰）。
//
// 三个要点：
// ① ESP32-S3 只支持 2.4 GHz。5 GHz 的 SSID 扫不到 —— 不是信号弱，是硬件不支持，
//    扫不出来。家里路由开了"双频合一"的话，2.4G 和 5G 共用一个名字，照样能连，
//    但连上的一定是 2.4G 那个。
// ② 等待连接必须有超时。指南给的 `while (WiFi.status() != WL_CONNECTED)` 是死循环：
//    密码写错、路由踢人、信号太弱，板子就永远卡在那儿，串口一个点一直打，
//    看不出是"还在连"还是"连不上"。给 15 秒上限，超时自己重启。
// ③ 连上之后要能自己恢复。路由器重启、Wi-Fi 密码改了、车跑到信号边缘，
//    掉线是常态。用 WiFi.onEvent() 挂回调，掉线自动重连，不用人去按复位键。
//    对一辆脱离 USB 自己跑的车来说，这一步是必须的。

#include <WiFi.h>
#include <RgbCycle.h>

// ---------- 改成你家的网络 ----------
const char* SSID = "你的WiFi名";     // 必须是 2.4 GHz
const char* PASS = "你的密码";

// 连接超时：15 秒连不上就判定失败。扫描 + 认证 + DHCP 正常 3~8 秒，
// 15 秒足够覆盖路由器繁忙的情况，又不至于让人干等太久。
const unsigned long CONNECT_TIMEOUT_MS = 15000;

// 板载 RGB 的颜色语言：
//   蓝闪 = 正在连  绿 = 已连上  红 = 失败/掉线
// 车以后脱离 USB 就没有串口了，灯是唯一的状态输出。从第一天就建立这套映射。

// 重连节流：掉线后不能每 loop 都调 begin()，1 秒内只试一次
unsigned long lastReconnectMs = 0;
const unsigned long RECONNECT_INTERVAL_MS = 1000;

// 心跳：连上之后每 5 秒报一次 RSSI，用来判断信号有没有在恶化
unsigned long lastHeartbeatMs = 0;
const unsigned long HEARTBEAT_MS = 5000;

bool connected = false;

// 人类可读的加密方式（比打印一个数字枚举有用）
const char* authName(uint8_t authType) {
  switch (authType) {
    case WIFI_AUTH_OPEN:            return "开放（无密码）";
    case WIFI_AUTH_WEP:             return "WEP（已淘汰）";
    case WIFI_AUTH_WPA_PSK:         return "WPA-PSK";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2-PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2 混合";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2 企业（需要账号）";
    default:                        return "未知";
  }
}

// RSSI 是负数 dBm，越接近 0 越好。给个口语化判据，省得每次查表
const char* rssiGrade(long rssi) {
  if (rssi >= -55) return "极好";
  if (rssi >= -67) return "好";
  if (rssi >= -75) return "可用";
  return "边缘，随时会掉";
}

void printNetInfo() {
  Serial.println("──────── 连接成功 ────────");
  Serial.printf("SSID      : %s\n", WiFi.SSID().c_str());
  Serial.printf("IP        : %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("子网掩码  : %s\n", WiFi.subnetMask().toString().c_str());
  Serial.printf("网关      : %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("DNS       : %s\n", WiFi.dnsIP().toString().c_str());
  Serial.printf("MAC       : %s\n", WiFi.macAddress().c_str());
  Serial.printf("信道      : %d（2.4G 共 11 个，互不重叠的只有 1/6/11）\n", WiFi.channel());
  Serial.printf("加密方式  : %s\n", authName(WiFi.encryptionType(0)));
  Serial.printf("信号 RSSI : %ld dBm（%s）\n", WiFi.RSSI(), rssiGrade(WiFi.RSSI()));
  Serial.println("──────────────────────────");
  // 后面所有实验都靠这个 IP 访问板子，单独再打一遍方便复制
  Serial.printf("记下这个 IP：%s\n", WiFi.localIP().toString().c_str());
}

// WiFi 事件回调。onEvent 比在 loop 里轮询 status 更早拿到状态变化，
// 而且掉线瞬间就能反应，不用等下一轮 loop
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      connected = true;
      RgbCycle::setColor(0, 255, 0);          // 绿：连上了
      printNetInfo();
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      // 断线原因码。这个数字是定位"为什么连不上"的关键：
      // 常见 201 = 没找到这个 SSID；202 = 认证失败（密码错）
      Serial.printf("⚠ 掉线，原因码 %d\n", info.wifi_sta_disconnected.reason);
      connected = false;
      RgbCycle::setColor(255, 0, 0);          // 红：掉了
      WiFi.reconnect();
      break;

    default:
      break;
  }
}

bool connectWiFi() {
  Serial.printf("连接 %s ...\n", SSID);
  WiFi.mode(WIFI_STA);          // 明确站模式。默认是 STA+AP，AP 那一路白占内存和功耗
  WiFi.begin(SSID, PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > CONNECT_TIMEOUT_MS) {
      // 超时不打"..."，直接给结论。让人猜"还在连还是在重试"最浪费时间
      Serial.printf("\n✗ %lu 毫秒内没连上。检查：① 是不是 2.4GHz ② 密码 ③ 信号\n",
                    CONNECT_TIMEOUT_MS);
      return false;
    }
    Serial.print(".");
    // 蓝灯闪烁表示"在连"，比一串点更直观
    RgbCycle::setColor(0, 0, ((millis() / 300) % 2) ? 255 : 0);
    delay(300);
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  RgbCycle::begin();
  RgbCycle::setColor(255, 0, 0);   // 上电 = 还没连，红

  // 打印芯片信息：确认 Wi-Fi 是这颗 S3 自己的，不是外接模块
  Serial.println("Day25 实验1：ESP32-S3 连 Wi-Fi");
  Serial.printf("芯片型号  : %s\n", ESP.getChipModel());
  Serial.printf("芯片版本  : %d  内核 %d 个\n", ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("Flash     : %u MB\n", ESP.getFlashChipSize() / 1024 / 1024);
  Serial.printf("IDF 版本  : %s\n", esp_get_idf_version());

  WiFi.onEvent(onWiFiEvent);       // 挂上再连，否则 GOT_IP 事件可能在连上之后才注册到

  if (!connectWiFi()) {
    // 连不上就 10 秒后重启重来。死等在那里没有任何信息产出，
    // 重启至少能排除"上一次连接过程把 Wi-Fi 状态机搞坏了"这种情况
    Serial.println("10 秒后重启重试");
    delay(10000);
    ESP.restart();
  }
}

void loop() {
  unsigned long now = millis();

  // 掉线自动重连（节流到 1 秒一次）
  if (!connected && WiFi.status() != WL_CONNECTED) {
    if (now - lastReconnectMs > RECONNECT_INTERVAL_MS) {
      lastReconnectMs = now;
      WiFi.begin(SSID, PASS);
    }
  }

  // 心跳：报 RSSI 和在线时长
  if (connected && now - lastHeartbeatMs > HEARTBEAT_MS) {
    lastHeartbeatMs = now;
    Serial.printf("在线 %lus｜RSSI %ld dBm（%s）｜IP %s\n",
                  now / 1000, WiFi.RSSI(), rssiGrade(WiFi.RSSI()),
                  WiFi.localIP().toString().c_str());
  }

  delay(100);
}
