#include <WiFi.h>
// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>
#include <config.h>
#include <BluetoothSerial.h>
#include <String.h>
#include <cstdio>
#include <Wire.h>
#include "OmniOperator.hpp"
#include "VL53L0XWrapper.hpp"

const float STRAIGHT_RATE = 1.0;
const float TURN_RATE = 1.0;
const int MAX_DIR = 60;
const int MIN_DISTANCE = 30;
const int MAX_DISTANCE = 100;
const bool DEBUG_ENABLE = true;
const uint8_t SHUT1=15,SHUT2=16,SHUT3=17;
const uint8_t ADDR1=0b0101001+1;
const uint8_t ADDR2=0b0101001+2;
const uint8_t ADDR3=0b0101001+3;

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
  Serial.println("G94 F70");
  delay(100);
  Serial.println("G1");
  delay(100);
  Serial.println("$120=3.500");
  delay(100);
  Serial.println("$121=3.500");
  delay(100);
  Serial.println("$122=3.500");
  delay(100);

}

BluetoothSerial serialBT;
OmniOperator omniopreator;
VL53L0XWrapper sensor1(SHUT1),sensor2(SHUT2),sensor3(SHUT3);

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");

  //モータドライバの初期化(Gコードの設定など)
  init_MD();
  serialBT.begin("esp32");
  omniopreator.init(1.0);//引数(range)で1命令当たりの各モータの最大移動量を指定．

  //センサーの初期化ほか
  Wire.begin(4,5,20000);
  sensor1.init(ADDR1);
  sensor2.init(ADDR2);
  sensor3.init(ADDR3);
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
  return;
}

struct Position{
  int direciton;
  int distance;
};

//TODO
int getHandPosition(){
  bool isSensor1Ranged = sensor1.isInnnerRange(MAX_DISTANCE);
  bool isSensor2Ranged = sensor2.isInnnerRange(MAX_DISTANCE);
  bool isSensor3Ranged = sensor3.isInnnerRange(MAX_DISTANCE);

  if(isSensor1Ranged == false&& isSensor2Ranged == false && isSensor3Ranged == false ){
    return OUT_OF_RANGE;
  }else if( isSensor1Ranged == true&& isSensor2Ranged == false && isSensor3Ranged == false ){
    return MORE_LEFT;
  }else if( isSensor1Ranged == true&& isSensor2Ranged == true && isSensor3Ranged == false ){
    return LEFT;
  }else if( isSensor1Ranged == false&& isSensor2Ranged == true && isSensor3Ranged == false ){
    return FRONT;
  }else if( isSensor1Ranged == false&& isSensor2Ranged == true && isSensor3Ranged == true ){
    return RIGHT;
  }else if( isSensor1Ranged == false&& isSensor2Ranged == false && isSensor3Ranged == true ){
    return MORE_RIGHT;
  }
  return OUT_OF_RANGE;
}

void moveMoter(int direction){
  float x=0,y=0,r=0;
  char sendStr[50];

  //回転成分がある場合は回転，回転成分がない場合のみ直進．
  switch(direction){
    case FRONT:
      x=0;
      y=STRAIGHT_RATE;
      break;
    case LEFT:
    case MORE_LEFT:
      r=-TURN_RATE;
      break;
    case RIGHT:
    case MORE_RIGHT:
      r=-TURN_RATE;
      break;
    case OUT_OF_RANGE:
    default:
      return;
  }
  omniopreator.calc_movement_value(x,y,r);

  sprintf(sendStr,"G91 X%f Y%f Z%f",omniopreator.get_top_motor_value(),omniopreator.get_right_motor_value(),omniopreator.get_left_motor_value());
  Serial.println(sendStr);
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
  Serial.println(sendStr);
  cnt++;
}

void loop()
{
  //Bluetooth経由でのコマンドを処理.受信していなければスルー
  if(serialBT.available()){
    String str = serialBT.readString();
    Command command(str);
    String extraInfo = command.getExtraInfo();
    if(DEBUG_ENABLE){
          Serial.print("Command:");
          Serial.println(command.getCommand());
          Serial.print("Info:");
          Serial.println(extraInfo);
      }
    switch (command.getCommand())
    {
    case 'f':
      if(extraInfo.equals("start")){
        state = PAUSE;
        pushButton();//この中にdelay入れる場合はstate不要かも
        state = RUNNING;
        if(DEBUG_ENABLE){
          Serial.println("start called");
        }
      }else if(extraInfo.equals("line_finished")){
        state = PAUSE;
        pushButton();
        state = RUNNING;
        if(DEBUG_ENABLE){
          Serial.println("line_finished");
        }
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
    //手の位置を検出するセンサの値を取得.中心が0,右ならプラス，左ならマイナス的なInt値で返す
    int direction = getHandPosition();
    
    //モータの回転量を反映
    moveMoter(direction);
  }

  // omniTest();
  delay(2000);
}

