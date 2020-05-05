#include <M5Stack.h>
#include <ros2arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define SSID        "SSID"            // お手元のWiFiアクセスポイント
#define SSID_PW     "PASSWORD"        // お手元のWiFiアクセスポイントパスワード
#define AGENT_IP    "192.168.XXX.XXX" // Micro XRCE-DDS-Agentを実行するパソコンのIPアドレス
#define AGENT_PORT  (2018)            // ポート番号（Agentと合わせる値）
#define PUB_FREQ    (100)

static WiFiUDP udp;                   // WiFi UDPクラスの実体

// ボタンPublisherコールバック関数
  ros2::Publisher<std_msgs::Bool>* btnA_pub_;
void publishButton( std_msgs::Bool* msg, void* arg )
{
  bool value = false;                 // 戻り値
  char type = *((char*)arg);          // コールバック登録で設定したボタンの種類を受け取る
  std_msgs::Bool data;

  switch( type ){                     // ボタンの種類で取得する値を選択する
  case 'A':                           // BtnAの場合
    M5.BtnA.read();                   // M5StackボタンA読み込み関数
    value = (M5.BtnA.isPressed()?true:false);// ボタン状態で戻り値設定
    break;
  case 'B':                           // BtnBの場合
    M5.BtnB.read();                   // M5StackボタンB読み込み関数
    value = (M5.BtnB.isPressed()?true:false);// ボタン状態で戻り値設定
    break;
  case 'C':                           // BtnCの場合
    M5.BtnC.read();                   // M5StackボタンC読み込み関数
    value = (M5.BtnC.isPressed()?true:false);// ボタン状態で戻り値設定
    break;
  }
  msg->data = value;                  // Publishするメッセージ内容を更新
  data.data = value;
  btnA_pub_->publish( &data );
}

class M5ROS2_Button : public ros2::Node // ros2::Node機能を持ったボタンクラスを作成
{
public:
  M5ROS2_Button()                     // コンストラクタ
  : Node("m5stack_button")            // 初期化時にノード名を設定する
  {
    std_msgs::Bool test;
    // ボタンA状態Publisher（std_msgs::Bool型）を基底クラスのNode機能で作成する
    btnA_pub_ = this->createPublisher<std_msgs::Bool>("btnA");
    // ボタンA状態のPublish周期とデータ更新関数を登録（周期Hz、コールバック関数、パラメータ、Publisher）
//    this->createWallFreq(PUB_FREQ,(ros2::CallbackFunc)publishButton,(void*)("A"),btnA_pub_);
    // ボタンB状態Publisher（std_msgs::Bool型）を基底クラスのNode機能で作成する
    ros2::Publisher<std_msgs::Bool>* btnB_pub_ = this->createPublisher<std_msgs::Bool>("btnB");
    // ボタンB状態のPublish周期とデータ更新関数を登録（周期Hz、コールバック関数、パラメータ、Publisher）
    this->createWallFreq(PUB_FREQ,(ros2::CallbackFunc)publishButton,(void*)("B"),btnB_pub_);
    // ボタンC状態Publisher（std_msgs::Bool型）を基底クラスのNode機能で作成する
    ros2::Publisher<std_msgs::Bool>* btnC_pub_ = this->createPublisher<std_msgs::Bool>("btnC");
    // ボタンC状態のPublish周期とデータ更新関数を登録（周期Hz、コールバック関数、パラメータ、Publisher）
    this->createWallFreq(PUB_FREQ,(ros2::CallbackFunc)publishButton,(void*)("C"),btnC_pub_);
  }
};

// Arduinoのセットアップ関数
void setup() {
  M5.begin();                         // M5Stack機能の初期化
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextColor(WHITE, BLACK);
  WiFi.begin(SSID, SSID_PW);          // WiFi通信の開始（接続先SSID、接続先SSIDのパスワード）
  M5.Lcd.println("Waiting for connection...");
  while(WiFi.status() != WL_CONNECTED);// WiFiが通信開始するまで待つ
  M5.Lcd.println("  WiFi OK.");
  ros2::init(&udp, AGENT_IP, AGENT_PORT, false);
                                      // ROS2機能の初期化（通信先UDP、AGENTのIPアドレスとポート、TCP無効）
}

// Arduinoのメインループ関数
void loop() {
  static bool ros2_regist = false;
  static M5ROS2_Button button_node;   // ボタンノードクラスをstaticで作成
  if( button_node.getNodeRegisteredState() ){ // ROS2接続できているか
    if( !ros2_regist ){               // 接続後初回のみ
      M5.Lcd.println("  ROS2 OK.");   // ROS2の接続状態を表示
      ros2_regist = true;
    }
    ros2::spin(&button_node);           // ボタンノードクラスのspin処理を毎回実行
  }
}
