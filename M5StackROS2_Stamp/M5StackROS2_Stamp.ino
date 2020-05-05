#include <M5Stack.h>
#include <ros2arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define SSID        "SSID"            // お手元のWiFiアクセスポイント
#define SSID_PW     "PASSWORD"        // お手元のWiFiアクセスポイントパスワード
#define AGENT_IP    "192.168.XXX.XXX" // Micro XRCE-DDS-Agentを実行するパソコンのIPアドレス
#define AGENT_PORT  (2018)            // ポート番号（Agentと合わせる値）

static WiFiUDP udp;                                 // WiFi UDPクラスの実体

// Stamp情報構造体
typedef struct {
    bool            status;                         // true:画像要求中
    uint16_t        index;                          // 要求中の分割番号
    const uint16_t  index_max;                      // 分割数
    uint16_t        seq;                            // シーケンス番号
    uint16_t        type;                           // 要求中のスタンプ種別
    uint8_t         data[1000*8];                   // 受信したスタンプデータ
} ST_STAMP_INFO;

// Subscriberコールバック関数
void subscribeStamp( m5stack_msgs::StampData* msg, void* arg )
{
    ST_STAMP_INFO* info = (ST_STAMP_INFO*)arg;
    uint16_t addr;

    if( info->status ){                             // 返事を待っている時だけ受け取る
        if( (msg->seq == info->seq) && (msg->index == info->index) && (msg->length > 0) ){
            // 正常なデータが届いたら格納する
            addr = (msg->index * msg->length);      // 格納先配列アドレスを作成
            memcpy( &info->data[addr], msg->data, msg->length );// 分割画像を格納
            ++info->index;                          // 次の分割番号へ
            info->status = false;                   // 要求受信済
            if( info->index >= info->index_max ){   // すべて受信したか
                info->index = 0;                    // 要求する分割番号初期化
            }
        }
    }
}

// M5StackStampクラス
class M5ROS2_Stamp : public ros2::Node              // ros2::Node機能を持ったクラスを作成
{
public:
    M5ROS2_Stamp()                                  // コンストラクタ
    : Node("m5stack_stamp_client")                  // 初期化時にノード名を設定する
    , stamp_width_(240), stamp_height_(240), stamp_top_(0), stamp_left(40)
    , stamp_type_a_(0), stamp_type_b_(1), stamp_type_c_(2)
    , stamp_info_( {0, 0, 8, 0, 0, {0}} )
    {
        // Publisher作成
        req_pub_ = this->createPublisher<m5stack_msgs::StampRequest>("stamp/request");
        // Subscriber作成
        this->createSubscriber<m5stack_msgs::StampData>("stamp/data"
                                                        , (ros2::CallbackFunc)subscribeStamp
                                                        , &stamp_info_);
    }
    enum EN_STAMP_TYPE : uint16_t {                 // スタンプ種別列挙
        EN_STAMP_A = 0,
        EN_STAMP_B,
        EN_STAMP_C
    };
    // スタンプ画像要求処理
    void reqStamp( EN_STAMP_TYPE type )
    {
        m5stack_msgs::StampRequest data;

        if( stamp_info_.index == 0 ){               // 要求途中でなければ
            M5.Lcd.fillRect(0,0,320,240,WHITE);     // 要求開始前に画面消去
            data.seq = ++stamp_info_.seq;           // シーケンス番号は常に可算
            data.index = stamp_info_.index = 0;     // 要求インデックス初期化
            data.type = stamp_info_.type = type;    // 要求するスタンプ種別を格納
            req_pub_->publish( &data );             // 要求データをPublish
            stamp_info_.status = true;              // 要求中
        }
    }
    // 周期処理
    void cycleStamp()
    {
        int16_t color;
        m5stack_msgs::StampRequest data;

        if( !stamp_info_.status ){                  // 受信待ちではない
            if( stamp_info_.index != 0 ){           // 要求途中なら
                // 次の分割データを要求する
                data.seq = ++stamp_info_.seq;       // シーケンス番号は常に可算
                data.index = stamp_info_.index;     // 要求インデックス設定
                data.type = stamp_info_.type;       // 要求するスタンプ種別を格納
                req_pub_->publish( &data );
                stamp_info_.status = true;
            }else{
                // 現在のスタンプ画像を描画する
                if( data.type == EN_STAMP_A ){
                    color = RED;                    // スタンプAは赤色
                }else if( data.type == EN_STAMP_B ){
                    color = BLUE;                   // スタンプBは青色
                }else{
                    color = GREEN;                  // スタンプCは緑色
                }
                M5.Lcd.drawXBitmap( 40, 0, stamp_info_.data, 240, 240, color );
            }
        }
    }
private:
    ros2::Publisher<m5stack_msgs::StampRequest>* req_pub_;
    const int16_t               stamp_width_, stamp_height_, stamp_top_, stamp_left;
    const uint16_t              stamp_type_a_, stamp_type_b_, stamp_type_c_;
    ST_STAMP_INFO               stamp_info_;
};

// Arduinoのセットアップ関数
void setup() {
    M5.begin();                                       // M5Stack機能の初期化
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextColor(WHITE, BLACK);
    WiFi.begin(SSID, SSID_PW);      // WiFi通信の開始（接続先SSID、接続先SSIDのパスワード）
    M5.Lcd.println("Waiting for connection...");
    while(WiFi.status() != WL_CONNECTED);             // WiFiが通信開始するまで待つ
    M5.Lcd.println("  WiFi OK.");
    ros2::init(&udp, AGENT_IP, AGENT_PORT, false);
                                    // ROS2機能の初期化（通信先UDP、AGENTのIPアドレスとポート、TCP無効）
}

// Arduinoのメインループ関数
void loop() {
    static bool ros2_regist = false;
    static M5ROS2_Stamp stamp_node;                 // Stampノードクラスをstaticで作成
    M5.update();
    if( stamp_node.getNodeRegisteredState() ){      // ROS2接続できているか
        if( !ros2_regist ){                         // 接続後初回のみ
            M5.Lcd.println("  ROS2 OK.");           // ROS2の接続状態を表示
            ros2_regist = true;
            delay(500);
            M5.Lcd.fillRect(0,0,320,240,WHITE);
        }
    }
    if( ros2_regist ){
        if( M5.BtnA.wasPressed() ){                 // ボタンAでスタンプAを要求
            stamp_node.reqStamp( M5ROS2_Stamp::EN_STAMP_A );
        }
        if( M5.BtnB.wasPressed() ){                 // ボタンBでスタンプBを要求
            stamp_node.reqStamp( M5ROS2_Stamp::EN_STAMP_B );
        }
        if( M5.BtnC.wasPressed() ){                 // ボタンCでスタンプCを要求
            stamp_node.reqStamp( M5ROS2_Stamp::EN_STAMP_C );
        }
        stamp_node.cycleStamp();                    // 周期処理実行
        ros2::spin(&stamp_node);                    // spin処理を実行
    }
}
