#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <SerialCommand.h>  // Thêm vào sketch thư viện Serial Command
// Gọi thư viện DHT11
#include "DHT.h"
// the light
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;
// ------------------------------------------------------
int AUTO = 1; // default is 1 -> auto; 0 -> manual
//------------cam bien nhiet do---------------------------
const int DHTPIN = 13;       //Đọc dữ liệu từ DHT11 ở chân 2 trên mạch Arduino
const int DHTTYPE = DHT11;  //Khai báo loại cảm biến, có 2 loại là DHT11 và DHT22
DHT dht(DHTPIN, DHTTYPE);
//--------------------------------------------------------
//-----------Mo-to----------------------------------------
int enA = 5;
#define in1 12
#define in2 11
int enB = 6;
#define in3 9
#define in4 8
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

  // anh sang
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  // On esp8266 devices you can select SCL and SDA pins using Wire.begin(D4, D3);
  Wire.begin();
  lightMeter.begin();

  //pinMode 2 đèn LED là OUTPUT
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);

  pinMode(DHTPIN, INPUT); // Đặt chân cảm biến mưa là INPUT, vì tín hiệu sẽ được truyền đến cho Arduino

  //----mo-to---
   pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  //------------

  // Một số hàm trong thư viện Serial Command
  sCmd.addCommand("STATUS", setStatus); // set up auto or not
  sCmd.addCommand("LED",   led); //Khi có lệnh LED thì sẽ thực thi hàm led
  sCmd.addCommand("MOTOR", runMotor); //Khi có lệnh MOTOR thì sẽ thực thi hàm runMotor
  sCmd.addCommand("FAN", runfan); //Khi có lệnh FAN thì sẽ thực thi hàm runfan
  Serial.println("Da san sang nhan lenh");
}

//------------------------------------
void motorChayThuan() { 
// cho chạy động cơ A  
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW); 
  analogWrite(enA, 230); // tốc độ 200, giá trị từ 0~255, băm xung PWM, mức cao 200/255. mức thấp 55/255. Tốc độ chạy khoảng (200/255)%.
  // chay 5s tat
//  delay(5000)
//  motorTat();
}
void motorChayNghich() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, 230); // tốc độ 200, giá trị từ 0~255, băm xung PWM, mức cao 200/255. mức thấp 55/255. Tốc độ chạy khoảng (200/255)%.
}
void quatChay() {
// cho chạy động cơ B 
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
   analogWrite(enB, 250);
}
void motorTat() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}
void quatTat() {
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

unsigned long chuky1 = 0;
void loop() {
  //Khởi tạo một chu kỳ lệnh, chu kỳ là 2000ms
  if (millis() - chuky1 > CHU_KY_1_LA_BAO_NHIEU) {
    chuky1 = millis();
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    //đọc giá trị cảm biến rồi in ra root
    uint16_t lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lux");
    int temp = dht.readTemperature(); //Đọc nhiệt độ
    int humidity = dht.readHumidity();    //Đọc độ ẩm

    // tu dong dieu khien
    if (AUTO == 1) {
      if(temp > 30) { // nhiet do
       quatChay();
      } else {
        quatTat();
      }
      if(humidity < 85) { // do am
        digitalWrite(red, 1);
        digitalWrite(blue, 1);
      } else {
        digitalWrite(red, 0);
        digitalWrite(blue, 0);
      }
      // cam biet anh sang
      // if(lux > 100) {
      //       
      // }
    }
  
    root["t"] = temp;
    root["h"] = humidity;
    root["l"] = lux;
    root["status"] = AUTO;

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

void setStatus() {
  Serial.println("STATUS");
  char *json = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
  Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json

  AUTO = root["status"];
}

// hàm led_red sẽ được thực thi khi gửi hàm LED_RED
void led() {
  Serial.println("LED");
  char *json = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
  Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json

  if(AUTO == 0) {
    int redStatus = root["led"][0];//json -> tham số root --> phần tử thứ 0. Đừng lo lắng nếu bạn không có phần tử này, không có bị lỗi đâu!
    int blueStatus = root["led"][1];//json -> tham số root --> phần tử thứ 0. Đừng lo lắng nếu bạn không có phần tử này, không có bị lỗi đâu!
  
    //kiểm thử giá trị
    Serial.print(F("redStatus "));
    Serial.println(redStatus);
    Serial.print(F("blueStatus "));
    Serial.println(blueStatus);
  
//    StaticJsonBuffer<200> jsonBuffer2;
//    JsonObject& root2 = jsonBuffer2.createObject();
//    root2["redStatus"] = redStatus;
//    root2["blueStatus"] = blueStatus;
//  
//    //Tạo một mảng trong JSON
//    JsonArray& data = root2.createNestedArray("data");
//    data.add(redStatus);
//    data.add(blueStatus);
//  
//  
//    //in ra cổng software serial để ESP8266 nhận
//    mySerial.print("LED_STATUS");   //gửi tên lệnh
//    mySerial.print('\r');           // gửi \r
//    root2.printTo(mySerial);        //gửi chuỗi JSON
//    mySerial.print('\r');           // gửi \r
//  
//    //in ra Serial để debug
//    root2.printTo(Serial); //Xuống dòng
  
    //xuất ra màn hình
    digitalWrite(red, redStatus);
    digitalWrite(blue, blueStatus);
  }
}

// Dieu khien motor voi lenh MOTOR
void runMotor() {
  Serial.println("MOTOR");
  char *json = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
  Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json

  if(AUTO == 0) {
    int role = root["role"];
    
    if (role == 1) {
       motorChayThuan();
    } else {
      if(role == 2) {
        motorChayNghich();
      } else {
        motorTat(); 
      }    
    }
  }
}
// Dieu khien Fan voi lenh FAN
void runfan() {
  Serial.println("FAN");
  char *json = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
  Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json

  if(AUTO == 0) {
    int role = root["role"];
    Serial.print("Quat dang: ");
    Serial.println(role);  
    
    if (role == 1) {
       quatChay();
    } else {
      quatTat(); 
    }   
  } 
}
