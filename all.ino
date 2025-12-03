
#include <WiFi.h> //подключаем библиотеку для работы с WiFi
#include <PubSubClient.h> // подключаем библиотеку для работы с MQTT
#include <ESP32Servo.h>

//using namespace std;

// define
Servo servo1;
const int ButtonPin = 21;   // Кнопка 
const int servoPin = 2;     // Servo pin GPIO2
const int endPin = 15;// Оптический концевик GPIO15
const int rele = 22; // Реле
const int clk = 5; // энкодер
const int dt = 18; // энкодер 
const int sw = 19; // энкодер, кнопка


// датчики
float d1 = 25;
float d2 = 100;
float d3 = 27;
float d4 = 24;
int m0 = 0;
int m1 = 5;
// Переменные для управления состоянием
bool isRotating = false;
bool lastButtonState = HIGH;

// переменные для энкодера
int counter = 0;
int lastCLK = HIGH;
int lastSW = HIGH;

// Замените значения SSID/Password на те, что вы указали при настройке роутера
const char* ssid = "MTEST";
const char* password = "mtestpass";
const char* espHostname = "espNik32";

// Адрес MQTT брокера и логин/пароль:
const char* mqtt_server = "192.168.42.244";
const char* mqttUsr = "fab";
const char* mqttPass = "fabfabfab";

WiFiClient espClient; 
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String inmsg = "";
int BUILTIN_LED = 19;

//String inmsg1 = "";

void setup_wifi() 
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.setHostname(espHostname);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  inmsg = "";
  // Serial.print("Message arrived [");
  Serial.print("] ");
  Serial.print(topic);
  
  Serial.print("сработал датчик0" + String(topic));
  //a = topic;
  for (int i = 0; i < length; i++) {
    inmsg += (char)payload[i];}
    //hd.push_back((char)payload[i]);
  if (String(topic) == "/humidifire/humiditysensor/01/"){
      d1 = inmsg.toFloat();
      }
    
  if (String(topic) == "/humidifire/humiditysensor/02/"){
      d2 = inmsg.toFloat();}
    
  if (String(topic) == "/humidifire/humiditysensor/03/"){
      d3 = inmsg.toFloat();}
    
  if (String(topic) == "/humidifire/humiditysensor/04/"){
      d4 = inmsg.toFloat();}
    
    // Serial.print((char)payload[i]);
  // Serial.println(inmsg);
  // Serial.println("from topic: " + inmsg);
  float  m = minimum(d1, d2, d3, d4);
  if (m == d4){
    m1 = 4;
    Serial.print("aaaaaaaaaa");}
  if (m == d1){
    m1 = 1;
    Serial.print("bbbbbbbbbbb");}
  if (m == d3){
    m1 = 3;
    Serial.print("cccccccccccc");}
  Serial.print(m1);
  Serial.print(m0);
  if (m0!=m1){
    init_without_button();
    rotate_servo(m);}

  if ((String)inmsg.c_str() == "off") {
    digitalWrite(BUILTIN_LED,  HIGH); // выключаем светодиод
    //Serial.println(String(inmsg.c_str()) + "dd");
    sendData("/miptfab/esp32led/ledState/", "OFF"); // отправляем состояние светодиода
  } else {//вот в этом месте приходят данные в порт если датчики тригерят
    digitalWrite(BUILTIN_LED, HIGH); // включаем светодиод
    delay(3000);
    digitalWrite(BUILTIN_LED, LOW);
    Serial.println(inmsg.c_str());
    sendData("/miptfab/esp32led/ledState/", "ON"); // отправляем состояние светодиода
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "esp32testClient-";
    clientId += String(random(0xffff), HEX);
    // пытаемся подключиться к брокеру MQTT
    if (client.connect(clientId.c_str(), mqttUsr, mqttPass)) {
      Serial.println("connected");
      // Как только подключились, сообщаем эту прекрасную весть...
      client.publish("/humidifire/state/", "connected");
      // ... ну и переподписываемся на нужный топик
      client.subscribe("/humidifire/humiditysensor/#");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Ждём 5 секунд перед следующей попыткой подключиться к брокеру MQTT
      delay(5000);
    }
  }
}



bool sendData(String topic, String data) {
  // String msgj = paramOne;
  if (!client.connected()) {
    reconnect();
  } else {
    client.publish(topic.c_str(), data.c_str());
     Serial.println(topic + " " + data);
  }
  return true;
}

void init_without_button(){
  bool isRotating = false;
  int endValue = digitalRead(endPin);   
  if (!isRotating) {
      // Запускаем вращение
      
      isRotating = true;
      // Подключаем серво
      if (!servo1.attached()) {
          servo1.attach(servoPin);
      }
      // Запускаем вращение
      servo1.write(94);
         
  } /*else {
      // Останавливаем вращение при повторном нажатии
      isRotating = false;
      // Останавливаем серво
      servo1.write(93); // Стоп
      delay(1000);
      servo1.detach();
      delay(1000);
  }*/

  bool flag = true;
  // Если серво вращается, проверяем концевик
  while (flag == true) {
      // Проверяем оптический концевик
      if (digitalRead(endPin) == HIGH) {
          // Концевик сработал - останавливаем
          isRotating = false;
          servo1.write(93); // Стоп
          delay(1000);
          servo1.detach();
          delay(2000);
          flag = false;
          //lastButtonState = HIGH;
      } else {
          // Продолжаем вращение
          delay(50);
      }
  }
  
  delay(100);
}


float rotate_servo(float d){
  int endValue = digitalRead(endPin);
  if (!servo1.attached()) {
              servo1.attach(servoPin);
          }
  if (d == d4){
    m0 = 4;
    Serial.print("Четвертый");
    Serial.print(d4);
    servo1.write(94);
    delay(2250);
    servo1.write(93); // Стоп
    delay(1000);
    servo1.detach();
    delay(1000);
  }

  

  if (d == d3){
    m0 = 3;
    Serial.print("Третий");
    servo1.write(94);
    delay(1125);
    servo1.write(93); // Стоп
    delay(1000);
    servo1.detach();
    delay(1000);
  }

  if (d == d1){
    m0 = 1;
    Serial.print("Первый");
    servo1.write(94);
    delay(5500);
    servo1.write(93); // Стоп
    delay(1000);
    servo1.detach();
    delay(1000);
  }

  return 0;
}



/*void rotate_servo_4(){
  int endValue = digitalRead(endPin);
  if (!servo1.attached()) {
              servo1.attach(servoPin);
          }
  servo1.write(94);
  delay(2250);
  servo1.write(93); // Стоп
          delay(1000);
          servo1.detach();
          delay(1000);
  
  }

void rotate_servo_1(){
int endValue = digitalRead(endPin);
if (!servo1.attached()) {
            servo1.attach(servoPin);
        }
servo1.write(94);
delay(3375);
servo1.write(93); // Стоп
        delay(1000);
        servo1.detach();
        delay(1000);

}

void rotate_servo_3(){
  int endValue = digitalRead(endPin);
  if (!servo1.attached()) {
              servo1.attach(servoPin);
          }
  servo1.write(94);
  delay(1125);
  servo1.write(93); // Стоп
          delay(1000);
          servo1.detach();
          delay(1000);
  
  }
  */
void init(){
  int buttonState = digitalRead(ButtonPin);
  int endValue = digitalRead(endPin);
  //digitalWrite(rele, HIGH);
  // Если кнопка нажата 
  if (buttonState == LOW && lastButtonState == HIGH) {
      
      if (!isRotating) {
          // Запускаем вращение
          
          isRotating = true;
          // Подключаем серво
          if (!servo1.attached()) {
              servo1.attach(servoPin);
          }
          // Запускаем вращение
          servo1.write(94);
          
          
          
      } else {
          // Останавливаем вращение при повторном нажатии
          isRotating = false;
          // Останавливаем серво
          servo1.write(93); // Стоп
          delay(1000);
          servo1.detach();
          delay(1000);
      }
  }
  lastButtonState = buttonState;
  
  // Если серво вращается, проверяем концевик
  if (isRotating) {
      // Проверяем оптический концевик
      if (digitalRead(endPin) == HIGH) {
          // Концевик сработал - останавливаем
          isRotating = false;
          servo1.write(93); // Стоп
          delay(1000);
          servo1.detach();
          delay(2000);
          //lastButtonState = HIGH;
      } else {
          // Продолжаем вращение
          delay(50);
      }
  }
  
  delay(100);
}
float minimum(float d1,float d2,float d3,float d4){
  float m = 100;
  if (d1<m){m = d1;}
  if (d2<m){m = d2;}
  if (d3<m){m = d3;}
  if (d4<m){m = d4;}
  return m;
  }
void encoder(){
  // Чтение энкодера
  int currentCLK = digitalRead(clk);
  if (currentCLK != lastCLK && currentCLK == LOW) {
    if (digitalRead(dt) == HIGH) {
      counter++;
    } else {
      counter--;
    }
    Serial.print("Count: ");
    Serial.println(counter);
  }
  lastCLK = currentCLK;
  
  // Обработка кнопки
  int currentSW = digitalRead(sw);
  if (currentSW == LOW && lastSW == HIGH) {
    delay(50); // Защита от дребезга
    counter = 0;
    Serial.println("Reset to 0");
    delay(300);
  }
  lastSW = currentSW;
  
  delay(1);
}

void setup() {
  Serial.begin(115200);
  //wifi
  pinMode(BUILTIN_LED, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(2500);
  digitalWrite(BUILTIN_LED, LOW);

  // servo
  servo1.attach(servoPin);
  pinMode(endPin, INPUT_PULLUP);     // Концевик с подтяжкой
  pinMode(ButtonPin, INPUT_PULLUP);

  // rele
  pinMode(rele, OUTPUT);
  digitalWrite(rele, LOW); // Выключить реле при старте

  // encoder

  pinMode(clk, INPUT_PULLUP);
  pinMode(dt, INPUT_PULLUP);
  pinMode(sw, INPUT_PULLUP);

  pinMode(ButtonPin, INPUT_PULLUP);

  digitalWrite(rele, HIGH); // Подать питание
  delay(1000); 
  }

void loop() {
  // WIFI
  /*if (touchRead(4)<20){
    Serial.println(touchRead(4));
    sendData("/humidifire/goto/", (String)touchRead(4));
  }*/
  
  if (!client.connected()) {
    reconnect();
    //sendData("/humidifire/state/", "up");
  }
  client.loop();
  //client.publish("/humidifire/humiditysensor/01/", "355");
  //sendData("/humidifire/state/", "up");
  // INIT
  init();
//float  m = minimum(d1, d2, d3, d4);
  //rotate_servo(m);
  //delay(10000);
  //init_without_button();
  //delay(1000);
  
  
  // включение увлажнителя
  //digitalWrite(rele, HIGH); // Подать питание
  //delay(1000); 
  //digitalWrite(rele, LOW); // Подать питание
  //delay(5000);
  // энкодер
  //encoder();
  
  /*if (digitalRead(endPin) == HIGH) {
    rotate_servo_3();
    }*/

  
  
}
