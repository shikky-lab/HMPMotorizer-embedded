#include <WiFi.h>
// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>
#include <config.h>
#include <BluetoothSerial.h>
#include <String.h>
#include <cstdio>

const int STRAIGHT_SPEED = 100;
const int TURN_SPEED = 100;
const int MAX_DIR = 60;
const int MIN_DISTANCE = 30;
const int MAX_DISTANCE = 100;

enum State{
  WAITING,
  RUNNING,
  PAUSE
};

int state = WAITING;

void init_MD(){

}

BluetoothSerial serialBT;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");

  //モータドライバの初期化(Gコードの設定など)
  init_MD();
  serialBT.begin("esp32");
}

class Command{
  private:
    char command;
    String extraInfo;
   public:
    Command(String str){
      command = str.c_str[0];
      extraInfo = str.substring(2,str.length-1);
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
  int target_motor_A=0,target_motor_B=0,target_motor_C=0;

  char sendStr[50];

  //回転成分がある場合は回転，回転成分がない場合のみ直進．
  if(handPoistion.direciton == 0){
    if( handPoistion.direciton < 0){
      target_motor_A=target_motor_B=target_motor_C=-TURN_SPEED;
    }else if(direction > 0){
      target_motor_A=target_motor_B=target_motor_C=TURN_SPEED;
    }
  }else{
    if(MIN_DISTANCE<handPoistion.distance && handPoistion.distance <STRAIGHT_SPEED){
      target_motor_B=target_motor_C=STRAIGHT_SPEED;
    }
  }

  sprintf(sendStr,"G91 X%dY%dZ%d\0",target_motor_A,target_motor_B,target_motor_C);
  Serial.println(sendStr);
}

void loop()
{
  //Bluetooth経由でのコマンドを処理.受信していなければスルー
  if(serialBT.available()){
    String str = serialBT.readString();
    Command command(str);
    String extraInfo = command.getExtraInfo();
    switch (command.getCommand())
    {
    case 'f':
      if(extraInfo.equals("start")){
        state = PAUSE;
        pushButton();//この中にdelay入れる場合はstate不要かも
        state = RUNNING;
      }else if(extraInfo.equals("line_finished")){
        state = PAUSE;
        pushButton();
        state = RUNNING;
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
}

