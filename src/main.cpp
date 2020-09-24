#include <WiFi.h>
// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>
#include <config.h>
#include <BluetoothSerial.h>
#include <String.h>
#include <cstdio>
#include <Wire.h>
#include <M5Stack.h>
#include "OmniOperator.hpp"
#include "VL53L0XWrapper.hpp"

const float STRAIGHT_RATE = 0.05;
// const float STRAIGHT_RATE = 0.1;
const float TURN_RATE = 0.05;
const int MAX_DIR = 60;
const int MIN_DISTANCE = 25;
const int MAX_DISTANCE = 100;
const bool DEBUG_ENABLE = true;
// const uint8_t SHUT1=18,SHUT2=17,SHUT3=19;
const uint8_t SHUT1=22,SHUT2=21,SHUT3=2;
// const uint8_t SDA_PIN=21,SCL_PIN=22;
const uint8_t SDA_PIN=3,SCL_PIN=13;
const uint8_t ADDR1=0b0101001+1;
const uint8_t ADDR2=0b0101001+2;
const uint8_t ADDR3=0b0101001+3;
const uint8_t SERVO_PIN=26;

const uint16_t LCD_HEIGHT = 240;
const uint16_t LCD_WIDTH = 320;
const uint16_t LCD_LEFT_X_POS = (LCD_WIDTH/4);
const uint16_t LCD_LEFT_Y_POS = 10;
const uint16_t LCD_CENTER_X_POS = (LCD_WIDTH/2);
const uint16_t LCD_CENTER_Y_POS = 10;
const uint16_t LCD_RIGHT_X_POS = (LCD_WIDTH*3/4);
const uint16_t LCD_RIGHT_Y_POS = 10;
const uint16_t LCD_DISPLAY_X_POS = LCD_WIDTH/2;
const uint16_t LCD_DISPLAY_Y_POS = LCD_HEIGHT/2;
const uint16_t LCD_BT_LEFT_X_POS = (LCD_WIDTH/4);
const uint16_t LCD_BT_LEFT_Y_POS = LCD_HEIGHT*3/4;
const uint16_t LCD_BT_CENTER_X_POS = (LCD_WIDTH/2);
const uint16_t LCD_BT_CENTER_Y_POS = LCD_HEIGHT*3/4;

enum State{
  WAITING,
  RUNNING,
  PAUSE
};
int state = WAITING;

enum DIRECTION{
  OUT_OF_RANGE,
  MORE_LEFT,
  LEFT,
  FRONT,
  RIGHT,
  MORE_RIGHT
};
int direction = OUT_OF_RANGE;

void init_MD(){
  Serial2.println("G94 F40");
  delay(100);
  Serial2.println("G1");
  delay(100);
  Serial2.println("$120=1.000");
  delay(100);
  Serial2.println("$121=1.000");
  delay(100);
  Serial2.println("$122=1.000");
  delay(100);

}

BluetoothSerial serialBT;
OmniOperator omniopreator;
VL53L0XWrapper sensor1(SHUT1),sensor2(SHUT2),sensor3(SHUT3);

void setup()
{
  Serial2.begin(115200);
  Serial2.println("Booting");

  //モータドライバの初期化(Gコードの設定など)
  init_MD();
  serialBT.begin("esp32");
  omniopreator.init(1.0);//引数(range)で1命令当たりの各モータの最大移動量を指定．

  //LCDの初期化
  M5.lcd.begin();
  M5.lcd.setTextSize(2);
  M5.Lcd.setTextDatum(4);

  //センサーの初期化ほか
  // Serial2.println("wait 5000");
  // delay(5000);
  Serial2.println("start init");
  Wire.begin(SDA_PIN,SCL_PIN,2000);
  int init_result1=0,init_result2=0,init_result3=0;
  init_result1 = sensor1.init(ADDR1);
  init_result2=sensor2.init(ADDR2);
  init_result3=sensor3.init(ADDR3);
  if(init_result1!=0 || init_result2!=0||init_result3!=0){
    m5.lcd.printf("ToF initialization error occured!!\nPlease check connection and restart");
    while(1);
  }
  ledcSetup(0, 50, 10);  // 0ch 50 Hz 10bit resolution
  ledcAttachPin(SERVO_PIN, 0); // pinアサイン, 0ch
  ledcWrite(0, 75);// (26/1024)*20ms ≒ 0.5 ms  (-90°)(123/1024)*20ms ≒ 2.4 ms (+90°)
     delay(100);
  //ledcWrite(0, 30);//戻すときはこれくらい
}

class Command{
  private:
    char command;
    String extraInfo;
   public:
    Command(String str){
      command = str.c_str()[0];
      extraInfo = str.substring(2,str.length()-1);
    }
    char getCommand(){
      return command;
    }
    String getExtraInfo(){
      return extraInfo;
    }
};

//HMPのボタンを押すメソッド
void pushButton(){
  ledcWrite(0, 30);
  delay(500);
  ledcWrite(0, 75);// (26/1024)*20ms ≒ 0.5 ms  (-90°)(123/1024)*20ms ≒ 2.4 ms (+90°)
  delay(100);
  return;
}

struct Position{
  int direciton;
  int distance;
};

//TODO
int getHandPosition(){
  bool isSensor1Ranged = sensor1.isInnnerRange(MAX_DISTANCE,MIN_DISTANCE);
  M5.Lcd.setCursor(LCD_LEFT_X_POS,LCD_LEFT_Y_POS);
  M5.Lcd.printf("%05d",sensor1.getDist());
  bool isSensor2Ranged = sensor2.isInnnerRange(MAX_DISTANCE,MIN_DISTANCE);
  M5.Lcd.setCursor(LCD_CENTER_X_POS,LCD_CENTER_Y_POS);
  M5.Lcd.printf("%05d",sensor2.getDist());
  bool isSensor3Ranged = sensor3.isInnnerRange(MAX_DISTANCE,MIN_DISTANCE);
  M5.Lcd.setCursor(LCD_RIGHT_X_POS,LCD_RIGHT_Y_POS);
  M5.Lcd.printf("%05d",sensor3.getDist());

  M5.Lcd.setCursor(LCD_DISPLAY_X_POS,LCD_DISPLAY_Y_POS);
  M5.Lcd.print("            ");
  M5.Lcd.setCursor(LCD_DISPLAY_X_POS,LCD_DISPLAY_Y_POS);
  if(isSensor1Ranged == false&& isSensor2Ranged == false && isSensor3Ranged == false ){
    M5.Lcd.print("OUT OF RANGE");
    return OUT_OF_RANGE;
  }else if( isSensor1Ranged == true&& isSensor2Ranged == false && isSensor3Ranged == false ){
    M5.Lcd.print("MORE LEFT");
    return MORE_LEFT;
  }else if( isSensor1Ranged == true&& isSensor2Ranged == true && isSensor3Ranged == false ){
    M5.Lcd.print("LEFT");
    return LEFT;
  }else if( isSensor1Ranged == false&& isSensor2Ranged == true && isSensor3Ranged == false ){
    M5.Lcd.print("FRONT");
    return FRONT;
  }else if( isSensor1Ranged == false&& isSensor2Ranged == true && isSensor3Ranged == true ){
    M5.Lcd.print("RIGHT");
    return RIGHT;
  }else if( isSensor1Ranged == false&& isSensor2Ranged == false && isSensor3Ranged == true ){
    M5.Lcd.print("MORE RIGHT");
    return MORE_RIGHT;
  }else{
    M5.Lcd.print("OTHER STATE");
    return OUT_OF_RANGE;
  }
  return OUT_OF_RANGE;
}

void moveMoter(int direction){
  float x=0,y=0,r=0;
  char sendStr[50];

  //回転成分がある場合は回転，回転成分がない場合のみ直進．
  switch(direction){
    case FRONT:
      x=-STRAIGHT_RATE;
      y=0;
      break;
    case LEFT:
    case MORE_LEFT:
      x=-STRAIGHT_RATE*0.5;
      r=TURN_RATE;
      break;
    case RIGHT:
    case MORE_RIGHT:
      x=-STRAIGHT_RATE*0.5;
      r=-TURN_RATE;
      break;
    case OUT_OF_RANGE:
    default:
      x=y=r=0.0;
      return;
  }
  omniopreator.calc_movement_value(x,y,r);
  sprintf(sendStr,"G91 X%f Y%f Z%f",omniopreator.get_top_motor_value(),omniopreator.get_right_motor_value(),omniopreator.get_left_motor_value());
  Serial2.println(sendStr);
}

void omniTest(){
  static int cnt=0;
  float x=0.,y=0,r=0.;

  switch(cnt%5){
    case 0:
      x=1;
      y=0;
      r=0;
      break;
    case 1:
      x=0;
      y=1;
      r=0;
      break;
    case 2:
      x=0;
      y=0;
      r=1;
      break;
    case 3:
      x=0;
      y=0;
      r=-1;
      break;
    case 4:
      x=-1;
      y=-1;
      r=0;
      break;
  }
  char sendStr[50];
  omniopreator.calc_movement_value(x,y,r);
  sprintf(sendStr,"G91 X%f Y%f Z%f",omniopreator.get_top_motor_value(),omniopreator.get_right_motor_value(),omniopreator.get_left_motor_value());
  Serial2.println(sendStr);
  cnt++;
}

void loop()
{
  //Bluetooth経由でのコマンドを処理.受信していなければスルー
  if(serialBT.available()){
    String str = serialBT.readString();
    Command command(str);
    String extraInfo = command.getExtraInfo();
    // if(DEBUG_ENABLE){
    //     Serial2.print("Command:");
    //     Serial2.println(command.getCommand());
    //     Serial2.print("info:");
    //     Serial2.println(command.getExtraInfo());
    //     // M5.Lcd.setCursor(LCD_BT_LEFT_X_POS,LCD_BT_LEFT_Y_POS);
    //     // M5.Lcd.print("                    ");
    //     // M5.Lcd.print("Command:");
    //     // M5.Lcd.printf(command.getCommand());
    //     // M5.Lcd.setCursor(LCD_BT_CENTER_X_POS,LCD_BT_CENTER_Y_POS);
    //     // M5.Lcd.print("                    ");
    //     // M5.Lcd.print("Info:");
    //     // M5.Lcd.println(extraInfo);
    //   }
    switch (command.getCommand())
    {
    case 'f':
      if(extraInfo.equals("start")){
        M5.Lcd.setCursor(LCD_BT_LEFT_X_POS,LCD_BT_LEFT_Y_POS);
        M5.Lcd.print("                    ");
        M5.Lcd.setCursor(LCD_BT_LEFT_X_POS,LCD_BT_LEFT_Y_POS);
        M5.Lcd.print("start called");

        state = PAUSE;
        pushButton();//この中にdelay入れる場合はstate不要かも
        state = RUNNING;


      }else if(extraInfo.equals("line_finished")){
        M5.Lcd.setCursor(LCD_BT_LEFT_X_POS,LCD_BT_LEFT_Y_POS);
        M5.Lcd.print("                    ");
        M5.Lcd.setCursor(LCD_BT_LEFT_X_POS,LCD_BT_LEFT_Y_POS);
        M5.Lcd.print("line finished");
        // state = PAUSE;
        // pushButton();
        // state = RUNNING;

      }else if(extraInfo.equals("end")){
        state=WAITING;
      }
      break;
    default:
      break;
    }
    // serialBT.println(str);
  }

  if(state == RUNNING || state == WAITING ){
    //手の位置を検出するセンサの値を取得.DIRECTIONの列挙型
    int direction = getHandPosition();
    
    //モータの回転量を反映
    moveMoter(direction);
  }

  // omniTest();
  delay(30);
}

