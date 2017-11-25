#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <SerialCommand.h>  // Thêm vào sketch thư viện Serial Command
// Gọi thư viện DHT11
#include "DHT.h"

//------------cam bien nhiet do---------------------------
const int DHTPIN = 13;       //Đọc dữ liệu từ DHT11 ở chân 2 trên mạch Arduino
const int DHTTYPE = DHT11;  //Khai báo loại cảm biến, có 2 loại là DHT11 và DHT22
DHT dht(DHTPIN, DHTTYPE);
//--------------------------------------------------------
//-----------Mo-to----------------------------------------
#define IN1 12
#define IN2 11
#define IN3 9
#define IN4 8
#define MAX_SPEED 255 //từ 0-255
#define MIN_SPEED 0
//--------------------------------------------------------

const byte RX = 3;          // Chân 3 được dùng làm chân RX
const byte TX = 2;          // Chân 2 được dùng làm chân TX

SoftwareSerial mySerial = SoftwareSerial(RX, TX);


SerialCommand sCmd(mySerial); // Khai báo biến sử dụng thư viện Serial Command

int red = 4, blue = 5; // led đỏ đối vô digital 4, led xanh đối vô digital 5

const unsigned long CHU_KY_1_LA_BAO_NHIEU = 2000UL; //Cứ sau 2000ms = 2s thì chu kỳ lặp lại

void setup() {
  //Khởi tạo Serial ở baudrate 57600 để debug ở serial monitor
  Serial.begin(57600);
  dht.begin();         // Khởi động cảm biến

  //Khởi tạo Serial ở baudrate 57600 cho cổng Serial thứ hai, dùng cho việc kết nối với ESP8266
  mySerial.begin(57600);

  //pinMode 2 đèn LED là OUTPUT
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);

  pinMode(DHTPIN, INPUT); // Đặt chân cảm biến mưa là INPUT, vì tín hiệu sẽ được truyền đến cho Arduino

  //----mo-to---
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  //------------

  // Một số hàm trong thư viện Serial Command
  sCmd.addCommand("LED",   led); //Khi có lệnh LED thì sẽ thực thi hàm led
  sCmd.addCommand("MOTOR", runMotor); //Khi có lệnh LED thì sẽ thực thi hàm led
  Serial.println("Da san sang nhan lenh");
}

//------------------------------------
void motor_1_Dung() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void motor_2_Dung() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void motor_1_Tien(int speed) { //speed: từ 0 - MAX_SPEED
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);//đảm báo giá trị nằm trong một khoảng từ 0 - MAX_SPEED - http://arduino.vn/reference/constrain
  digitalWrite(IN1, HIGH);// chân này không có PWM
  analogWrite(IN2, 255 - speed);
}

void motor_1_Lui(int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);//đảm báo giá trị nằm trong một khoảng từ 0 - MAX_SPEED - http://arduino.vn/reference/constrain
  digitalWrite(IN1, LOW);// chân này không có PWM
  analogWrite(IN2, speed);
}

void motor_2_Tien(int speed) { //speed: từ 0 - MAX_SPEED
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);//đảm báo giá trị nằm trong một khoảng từ 0 - MAX_SPEED - http://arduino.vn/reference/constrain
  analogWrite(IN3, speed);
  digitalWrite(IN4, LOW);// chân này không có PWM
}

void motor_2_Lui(int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);//đảm báo giá trị nằm trong một khoảng từ 0 - MAX_SPEED - http://arduino.vn/reference/constrain
  analogWrite(IN4, 255 - speed);
  digitalWrite(IN3, HIGH);// chân này không có PWM
}
//------------------------------------

unsigned long chuky1 = 0;
void loop() {
  //Khởi tạo một chu kỳ lệnh, chu kỳ là 2000ms
  if (millis() - chuky1 > CHU_KY_1_LA_BAO_NHIEU) {
    chuky1 = millis();
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    //đọc giá trị cảm biến rồi in ra root
    root["t"] = dht.readTemperature(); //Đọc nhiệt độ
    root["h"] = dht.readHumidity();    //Đọc độ ẩm

    Serial.print("Nhiet do: ");
    Serial.println(dht.readTemperature());               //Xuất nhiệt độ
    //Gửi đi hoy!
    //in ra cổng software serial để ESP8266 nhận
    mySerial.print("TEM_STATUS");   //gửi tên lệnh
    mySerial.print('\r');           // gửi \r
    root.printTo(mySerial);        //gửi chuỗi JSON
    mySerial.print('\r');

    //in ra Serial để debug
    root.printTo(Serial); //Xuống dòng
  }
  sCmd.readSerial();
  //Bạn không cần phải thêm bất kỳ dòng code nào trong hàm loop này cả
}

// hàm led_red sẽ được thực thi khi gửi hàm LED_RED
void led() {
  Serial.println("LED");
  char *json = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
  Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json

  int redStatus = root["led"][0];//json -> tham số root --> phần tử thứ 0. Đừng lo lắng nếu bạn không có phần tử này, không có bị lỗi đâu!
  int blueStatus = root["led"][1];//json -> tham số root --> phần tử thứ 0. Đừng lo lắng nếu bạn không có phần tử này, không có bị lỗi đâu!

  //kiểm thử giá trị
  Serial.print(F("redStatus "));
  Serial.println(redStatus);
  Serial.print(F("blueStatus "));
  Serial.println(blueStatus);

  StaticJsonBuffer<200> jsonBuffer2;
  JsonObject& root2 = jsonBuffer2.createObject();
  root2["redStatus"] = redStatus;
  root2["blueStatus"] = blueStatus;

  //Tạo một mảng trong JSON
  JsonArray& data = root2.createNestedArray("data");
  data.add(redStatus);
  data.add(blueStatus);


  //in ra cổng software serial để ESP8266 nhận
  mySerial.print("LED_STATUS");   //gửi tên lệnh
  mySerial.print('\r');           // gửi \r
  root2.printTo(mySerial);        //gửi chuỗi JSON
  mySerial.print('\r');           // gửi \r

  //in ra Serial để debug
  root2.printTo(Serial); //Xuống dòng

  //xuất ra màn hình
  digitalWrite(red, redStatus);
  digitalWrite(blue, blueStatus);
}

// Dieu khien motor voi lenh MOTOR
void runMotor() {
  Serial.println("MOTOR");
  char *json = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
  Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json

  int role = root["role"];
  if (role == 1) {
    motor_2_Lui(MAX_SPEED); //motor 2 lùi
  } else {
    motor_2_Dung();
  }
}
