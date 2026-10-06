#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <PubSubClient.h> 


Adafruit_MPU6050 mpu;
const int boxSwitchPin = 4;
int lastBoxState = HIGH;

const float SHOCK_THRESHOLD = 15.0; 


// ================= إعدادات MQTT =================
const char* mqttServer = "mqtt3.thingspeak.com";
const int mqttPort = 1883;
const char* mqttClientID = "GikjOBwOARUkLjYpECkmKjU"; 
const char* mqttUsername = "GikjOBwOARUkLjYpECkmKjU";   
const char* mqttPassword = "YBUz+Os/+1wXYxwv1xa1wMb7";   
// ================================================

// ================= إعدادات السحابة =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";
unsigned long myChannelNumber = 3499569; 
const char * myWriteAPIKey = "2HAOTN65SVZHS8DO";
// ===================================================

WiFiClient client;

PubSubClient mqttClient(client);

unsigned long lastSendTime = 0;

float maxAccelSinceLastSend = 0.0;
int eventCode = 0;

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // محاولة الاتصال باستخدام بيانات الاعتماد
    if (mqttClient.connect(mqttClientID, mqttUsername, mqttPassword)) {
      Serial.println("MQTT Connected Successfully!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(boxSwitchPin, INPUT_PULLUP);

  if (!mpu.begin()) {
    Serial.println("System Halt: Sensor missing.");
    while (1) { delay(10); }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  
  mqttClient.setServer(mqttServer, mqttPort);

  Serial.println("OmniTrack System Ready. Monitoring Started...");
}

void loop() {

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop(); 

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  

  float totalAccel = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));

  if(totalAccel > maxAccelSinceLastSend) {
    maxAccelSinceLastSend = totalAccel;
  }

  int currentBoxState = digitalRead(boxSwitchPin);
  
  // ============ نظام اكتشاف الأحداث (Event Detection) ============
  if (totalAccel > SHOCK_THRESHOLD && eventCode == 0) {
    Serial.println("[تنبيه] تم اكتشاف صدمة قوية!");
    eventCode = 1;
  }
  else if (a.acceleration.z < -5.0 && eventCode == 0) {
    Serial.println("[تنبيه] الصندوق مقلوب رأساً على عقب!");
    eventCode = 2;
  }
  else if (currentBoxState == LOW && lastBoxState == HIGH) {
    Serial.println("[تنبيه] تم فتح الصندوق بدون تصريح!");
    eventCode = 3;
  }
  
  lastBoxState = currentBoxState;


  if (millis() - lastSendTime > 15000) {
    // 1. تجهيز الرسالة بصيغة النص المترابط (URL Encoded) كما يطلبها ThingSpeak
    String payload = "field1=" + String(maxAccelSinceLastSend) + 
                     "&field2=" + String(eventCode) + 
                     "&field3=" + String(currentBoxState == LOW ? 1 : 0);
                     
    // 2. تجهيز مسار الإرسال (Topic) الذي يحتوي على رقم قناتك
    String topic = "channels/" + String(myChannelNumber) + "/publish";

    // 3. نشر الرسالة (Publish)
    if(mqttClient.publish(topic.c_str(), payload.c_str())) {
      Serial.println("✅ تم رفع السجل الأمني للسحابة بنجاح عبر MQTT!");
    } else {
      Serial.println("❌ فشل الإرسال عبر MQTT");
    }
    
    // تصفير المتغيرات لتبدأ الدورة من جديد
    lastSendTime = millis();
    maxAccelSinceLastSend = 0.0;
    eventCode = 0; 
  }

  
  delay(50); // تأخير صغير جداً لا يؤثر على الأداء
}
