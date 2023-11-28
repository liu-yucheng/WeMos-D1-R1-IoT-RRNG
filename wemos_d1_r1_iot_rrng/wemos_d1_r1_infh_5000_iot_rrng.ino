// WeMos D1 R1
// INFH 5000 Project 42
// Internet of Things Real Random Number Generator (IoT RRNG)

// Developers: Dayou Du, Yucheng Liu
// Emails: ddu487@connect.hkust-gz.edu.cn, yliu428@connect.hkust-gz.edu.cn

// Copyright (C) 2023 Dayou Du, GNU AGPL3/3+ license.

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#define SERIAL_BAUD_RATE 115200
#define OUT_PACKET_BYTE_COUNT 2048
#define REMOTE_PORTS_COUNT 6
#define WIFI_STATUS_PRINT_INTERVAL_MS 1000

const char ssid[] = "INFH-5000-Project-42";
const char password[] = "42istheanswer";

int ledOn = 1;
const int pwmPin = D2; // 将D2引脚设置为PWM输出

IPAddress localIP(192, 168, 1, 1);
IPAddress localGateway(192, 168, 1, 1);
IPAddress localSubnet(255, 255, 255, 0);

Ticker wifiStatusPrintTicker;

WiFiUDP localUDP;
unsigned int localPort = 50420;

long packetIndex = 0;
char outPacket[OUT_PACKET_BYTE_COUNT];
IPAddress remoteIP(192, 168, 1, 255);
// unsigned int remotePorts[REMOTE_PORTS_COUNT] = { 10420, 20420, 30420, 40420, 50420, 60420 };
unsigned int remotePorts[REMOTE_PORTS_COUNT] = { 50420 };
uint8_t pwm_dir = 0;
uint32_t pwmval = 100;

void setup();
void loop();

void setupSerial();
void setupLED();
void setupWiFi();
void setupWiFiStatusPrint();
void setupUDP();
void loopUDP();
void printWiFiStatus();
void toggleLED();
void setupPwm();

void setup() {
  setupPwm();
  setupSerial();
  setupLED();
  setupWiFi();
  setupWiFiStatusPrint();
  setupUDP();
}

void loop() {
  if (WiFi.softAPgetStationNum() > 0) {
    loopUDP();  // 只有当至少有一个客户端连接时才发送UDP数据
  }
  Serial.println("Waiting for the client...");
}

void setupPwm() {
  analogWriteFreq(1000);            // 频率设置为1kHz，即周期为1ms
  analogWriteRange(1000);           // 范围设置为1000，即占空比步长为1us
  analogWrite(pwmPin, pwmval);              // GPIO2 - D4 - LED
}

void setupSerial() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println();
  Serial.printf("Completed setting up serial\n");
}

void setupLED() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.printf("Completed setting up LED\n");
}

void setupWiFi() {
  Serial.printf("Will configure WiFi soft AP\n");
  bool successful = WiFi.softAPConfig(localIP, localGateway, localSubnet);
  Serial.printf(successful ? "Configuration successful\n" : "Configuration failed\n");

  Serial.printf("Will configure WiFi soft AP\n");
  // WiFi.softAP(ssid);
  // WiFi.softAP(ssid, password, channel, hidden, max_connection);
  successful = WiFi.softAP(ssid, password);
  Serial.printf(successful ? "Setup successful\n" : "Setup failed\n");

  Serial.printf("WiFi soft AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
  toggleLED();
}

void setupWiFiStatusPrint() {
  wifiStatusPrintTicker.attach_ms(WIFI_STATUS_PRINT_INTERVAL_MS, printWiFiStatus);
  Serial.printf("Completed setting up WiFi status printing\n");
  toggleLED();
}

void setupUDP() {
  localUDP.begin(localPort);
  Serial.printf("Began WiFi UDP; IP: %s; Port: %d\n", WiFi.localIP().toString().c_str(), localPort);
  toggleLED();
}

int getSensor() {
  int sum = 0;
  int sensorValue = 0;
  // 连续读取10次
  for (int i = 0; i < 10; i++) {
    sensorValue = analogRead(A0); // 读取A0引脚的模拟值
    sum += sensorValue; // 将读取的值累加到sum变量中
    delay(50); // 在每次读取之间稍作延迟
  }
  int avgSensor = sum / 10;
  return avgSensor;
}

void loopUDP() {
  int avgSensor = 0;
  while (1) {
    avgSensor = getSensor();
    if (avgSensor > 0 and avgSensor != 102 and avgSensor != 204) break; // 102, 204 为经常重复的值。。
  }
  // 在串口监视器上打印平均值
  Serial.printf("Average AnalogRead: %d\n", avgSensor);

  if( pwmval <= pwmval - 50 ) pwm_dir=1;        // pwmval降低至500后，方向为递增
  if( pwmval == pwmval + 50 ) pwm_dir=0;        // pwmval递增到1000后，方向改为递减
  if(pwm_dir) pwmval++;                         // dir==1  pwmval递增
  else pwmval--;                                // dir==0  pwmval递减
  Serial.print("PWM_val: ");
  Serial.println(pwmval);                       // 打印当前PWM值
  analogWrite(pwmPin, pwmval);                  // 修改占空比

  
  
  unsigned long timeMs = millis();
  Serial.printf("Will send data packet; timeMs: %ld\n", timeMs);
  sprintf(outPacket, "packetIndex %d randomNumber %d", packetIndex, avgSensor);
  int sent;

  for (int index = 0; index < REMOTE_PORTS_COUNT; index += 1) {
    
    localUDP.beginPacket(remoteIP, remotePorts[index]);
    localUDP.write((const unsigned char*)outPacket, strlen(outPacket));
    sent = localUDP.endPacket();

    Serial.printf("Sent UDP packet\n");
    Serial.printf("Sent: %s; Receiver IP: %s; Receiver port: %d\n", sent ? "true" : "false", remoteIP.toString().c_str(), remotePorts[index]);
    Serial.printf("\n____ Begin packet contents ____\n");
    Serial.printf("%s\n", outPacket);
    Serial.printf("^^^^ End packet contents ^^^^\n\n");
    Serial.printf("Sent data packet\n");

    delay(200);
  }

  packetIndex += 1;
}

void printWiFiStatus() {
  Serial.printf("WiFi soft AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
  toggleLED();
}

void toggleLED() {
  if (ledOn == 0) {
    digitalWrite(LED_BUILTIN, LOW);
    ledOn = 1;
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    ledOn = 0;
  }
}
