
#include <HTTPClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <Arduino_JSON.h>
#include <ESP32Servo.h>

const char* ssid ="AnAn";     
const char* password ="27082002";  
const int trig1 = 19, trig2 = 22;     // chân trig của HC-SR04
const int echo1 = 18, echo2 = 23;     // chân echo của HC-SR04
const int PIN_servo = 5;
bool checkNapThung = false;
int preTrashVal = 0;
Servo servo1;
//MQTT
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_USER "DucAnh"
#define MQTT_PASSWORD "25112002"
WiFiClient wifiClient;
PubSubClient client(wifiClient);
//END_MQTT

void setup()
{
  Serial.begin(115200);     // giao tiếp Serial với baudrate 115200
  pinMode(trig1,OUTPUT);   // chân trig sẽ phát tín hiệu
  pinMode(echo1,INPUT);    // chân echo sẽ nhận tín hiệu
  pinMode(trig2,OUTPUT);   // chân trig sẽ phát tín hiệu
  pinMode(echo2,INPUT);    // chân echo sẽ nhận tín hiệu
  servo1.attach(PIN_servo);
  //***********************setup wifi
  WiFi.begin(ssid,password);
  Serial.println("conecting");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  //END_wifi

  //MQTT
  client.setServer(MQTT_SERVER, MQTT_PORT );
  client.setCallback(callback);
  connect_to_broker();
}
int measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH) / 58.2;  // Chuyển đổi thời gian thành khoảng cách (đơn vị cm)
}
void loop()
{
  int distance1 = measureDistance(trig1,echo1);
  int distance2 = measureDistance(trig2,echo2);
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }

  /* In kết quả ra Serial Monitor */
  Serial.print("Sieu am mo nap: ");
  Serial.print(distance1);
  Serial.println("cm");
  if(distance1 < 15 && !checkNapThung){
    client.publish("PTIT/IoTN1/servo","1");
    checkNapThung=true;
  }
  if (distance1>15 && checkNapThung)
  {
    client.publish("PTIT/IoTN1/servo","0");
    checkNapThung=false;
  }
  Serial.print("Sieu am do rac: ");
  Serial.print(distance2);
  Serial.println("cm");
  if(abs(distance2 - preTrashVal) > 3){
    preTrashVal = distance2;
    char* res = intToString(distance2);
    client.publish("PTIT/IoTN1/dashboard", res);
  }
  delay(500);
}

void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe("PTIT/IoTN1/servo");
      client.subscribe("PTIT/IoTN1/mode");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
void callback(char* topic, byte *payload, unsigned int length) {
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  if(compareString(topic,"PTIT/IoTN1/servo")){
    if (*payload == '1'){
      openServo();
    }
    else if (*payload == '0'){
      closeServo();
    }
  }
  if(compareString(topic,"PTIT/IoTN1/mode")){
    if (*payload == '1'){
      setMode(1);
    }
    else if (*payload == '0'){
      setMode(0);
    }
  }

  
  
}
void openServo(){
  for(int i = 0; i <= 90 ; i +=10){
    servo1.write(i);
    delay(50);
  }
}

void closeServo(){
  for(int i = 90; i >= 0 ; i -= 10){
    servo1.write(i);
    delay(50); 
  }
}

int abs(int a){
  if(a > 0) return a;
  return -a;
}
char* intToString(int num) {
    // Dùng hàm sprintf để thực hiện chuyển đổi
    // sprintf sẽ ghi chuỗi được tạo vào một buffer và trả về địa chỉ của buffer đó
    char* str;
    int length = snprintf(NULL, 0, "%d", num); // Xác định độ dài chuỗi cần phải cấp phát

    // Cấp phát bộ nhớ cho chuỗi
    str = (char*)malloc(length + 1);

    // Thực hiện chuyển đổi số thành chuỗi
    snprintf(str, length + 1, "%d", num);

    return str;
}
bool compareString(const char* str1, const char* str2) {
  while (*str1 && (*str1 == *str2)) {
    str1++;
    str2++;
  }

  if (*str1 == '\0' && *str2 == '\0') {
    return true;
  }
  return false;
}
void setMode(int n){
  
}