/*
 * ライブラリをarduinoに反映する方法
 * 
 * step1: 各リンクからライブラリをインストールしたのちに、「スケッチ」->「ファイルを追加」->ダウンロードしたファイルを選択
 * step2: 「スケッチ」->「ライブラリをインクルード」->先ほどインストールしたライブラリ名が一番下に表示されているので、それを選択
 * 
 */

#include <TimerOne.h>          // 赤外線反射センサーを制御するためのライブラリ (https://files.seeedstudio.com/wiki/Grove-Infrared_Reflective_Sensor/res/TimerOne-ArduinoLib.zip)
#include <SparkFun_TB6612.h>   // motorを制御するためのライブラリ (https://github.com/sparkfun/SparkFun_TB6612FNG_Arduino_Library/archive/master.zip)
#include <rgb_lcd.h>           // LCDを制御するためのライブラリ   (https://github.com/Seeed-Studio/Grove_LCD_RGB_Backlight/archive/master.zip)

#define MAGNECTIC_SWITCH 2     // 磁気センサーのportを定義(今回はD2)

#define AIN1 8  //motor1の入力portを定義
#define AIN2 9  //motor1の入力portを定義
#define PWMA 5  //motor1の出力portを定義

#define BIN1 7  //motor2の入力portを定義
#define BIN2 3  //motor2の入力portを定義
#define PWMB 6  //motor2の出力portを定義

#define STBY 10

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;   //motor2の出力
const int offsetB = -1;  //motor2の出力

Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);  //motor1を定義
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);  //motor2を定義

rgb_lcd lcd;             // LCDを制御するためのクラスをlcdと命名(命名しないのであれば、lcdの箇所をrgb_lcdとすればよい)
const int colorR = 255;  // LCDの背景色をRGBで決定
const int colorG = 0;    // LCDを背景色をRGBで決定
const int colorB = 0;    // LCDを背景色をRGBで決定

int  count      = 0;      // 磁気を検知した回数
bool count_flag = false;  // 磁気を初めて検知したかどうか（磁気を検知し続けるためにloop処理を使っているので、ループ処理の1回目のみカウントアップするように）
int type        = 0;      // モーターの動きをtypeで管理している。詳細はturnOnLcdn参照 (1:両輪前進, 2:左に旋回, 3:右に旋回, 4:両輪後進)

void setup() 
{
    // MOTOR
    Timer1.initialize(100000);               // 赤外線反射センサーが検知する単位時間（ms）
    Timer1.attachInterrupt( controlMotor );  // attach the service routine here
  
    //LCD
    pinMode(MAGNECTIC_SWITCH, INPUT);    // 磁気センサーからの入力portを定義
    lcd.begin(16, 2);                    // LCDに表示される上限は32bitなので、表示される形を2行16列と定義
    lcd.setRGB(colorR, colorG, colorB);  // LCDの背景色を定義
    lcd.print("count is start!!");       // LCDに表示される文字（なくてもよい）
}

void loop()
{
    // 磁気検知すればturnOnLCDを、されなければturnOffLCDを実行
    if(isNearMagnet()) {
      turnOnLCD();
    } else {
      turnOffLCD(); 
    }
}

void controlMotor()
{
    // 赤外線反射センサーとモーターを利用
    // 進行方向に向かって左右を決めている
    // motor1:A1 左 motor2:A2 右
    // digitalRead(port)で、white = 0, black = 1としてコース上を走る車を制御
    if(digitalRead(A1) == 1  && digitalRead(A2) == 1) {
      type = 1;
      forward(motor1, motor2, 150);  // 両輪前進
    } else if(digitalRead(A1) == 0  && digitalRead(A2) == 1) {
      type = 2;
      left(motor1, motor2, 100);     // 左に旋回
    } else if(digitalRead(A1) == 1  && digitalRead(A2) == 0) {
      type = 3;
      right(motor1, motor2, 100);    // 右に旋回
    } else if(digitalRead(A1) == 0  && digitalRead(A2) == 0) {
      type = 4;
      back(motor1, motor2, 150);     // 両輪後進
    };
}

boolean isNearMagnet()
{
    // 磁気センサーを利用
    int sensorValue = digitalRead(MAGNECTIC_SWITCH);
    if(sensorValue == HIGH){
      return true;
    } else {
      return false;
    }
}

void turnOnLCD() 
{  
    // 磁気センサーを利用
    // loop処理での1回目のみカウントアップ
    // モーターを回しながらLCDを制御しようとすると、電池供給不足を起こし回路全体が止まってしまうことがあった
    // 対応として、カウントアップがあった際にモーターを止めてLCDを書き換え、それまでのモーターの動きを引き継ぐ
    // 引き継ぐためにモーターの動きをtypeで管理している
    // モーターが止まり、また動き出すまでの時間は十分に短いので無視して良い
    if(count_flag == false) {
      brake(motor1, motor2);
      count = count + 1; 
      lcd.setCursor(0, 1);
      lcd.print(count);
      
      if(type == 1 ){
        forward(motor1, motor2, 150);  // 両輪前進
      } else if(type == 2){
        left(motor1, motor2, 100);     // 左に旋回
      } else if(type == 3){
        right(motor1, motor2, 100);    // 右に旋回
      } else if(type == 4){
        back(motor1, motor2, 150);     // 両輪後進
      }
    }
    
    count_flag = true;
}

void turnOffLCD()
{
    // 磁気センサーが検知しなくなればfalseに変更
    count_flag = false;
}