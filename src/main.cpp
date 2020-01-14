#include <WiFi.h>
// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>
#include <config.h>
#include <BluetoothSerial.h>
#include <String.h>
#include <cstdio>
#include "OmniOperator.hpp"

const float STRAIGHT_RATE = 1.0;
const float TURN_RATE = 1.0;
const int MAX_DIR = 60;
const int MIN_DISTANCE = 30;
const int MAX_DISTANCE = 100;
const bool DEBUG_ENABLE = true;

enum State{
  WAITING,
  RUNNING,
  PAUSE
};

int state = WAITING;

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

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");

  //モータドライバの初期化(Gコードの設定など)
  init_MD();
  serialBT.begin("esp32");
  omniopreator.init(1.0);//引数(range)で1命令当たりの各モータの最大移動量を指定．
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
Position getHandPosition(){
  Position handPosition;
  handPosition.distance=0;
  handPosition.direciton=0;
  return handPosition;
}

void moveMoter(Position handPoistion){
  
  int direction = handPoistion.direciton;
  int distance = handPoistion.distance;
  float x=0,y=0,r=0;

  char sendStr[50];

  //回転成分がある場合は回転，回転成分がない場合のみ直進．
  if(handPoistion.direciton != 0){
    if( handPoistion.direciton < 0){
      r=-TURN_RATE;
    }else if(direction > 0){
      r=TURN_RATE;
    }
  }else{
    if(MIN_DISTANCE<handPoistion.distance && handPoistion.distance <MAX_DISTANCE){
      x=0;
      y=STRAIGHT_RATE;
    }
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
    Position handPosition = getHandPosition();
    
    //モータの回転量を反映
    moveMoter(handPosition);
  }

  // omniTest();
  delay(2000);
}

