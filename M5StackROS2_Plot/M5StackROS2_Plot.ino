#include <M5Stack.h>
#include <ros2arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define SSID        "SSID"            // お手元のWiFiアクセスポイント
#define SSID_PW     "PASSWORD"        // お手元のWiFiアクセスポイントパスワード
#define AGENT_IP    "192.168.XXX.XXX" // Micro XRCE-DDS-Agentを実行するパソコンのIPアドレス
#define AGENT_PORT  (2018)            // ポート番号（Agentと合わせる値）

static WiFiUDP udp;                                   // WiFi UDPクラスの実体

#define PLOT_WIDTH  (320)                             // 画面の幅
#define PLOT_HEIGHT (240)                             // 画面の高さ
#define PLOT_CENTER (120)                             // 画面の中心
#define PLOT_MAX    (PLOT_HEIGHT/2)
#define PLOT_MIN    (-(PLOT_HEIGHT/2))

// Subscriberコールバック関数
void subscribePlot( m5stack_msgs::Plot* msg, void* arg )
{
    int16_t* buff = (int16_t*)arg;
    memcpy( buff, msg->data, sizeof(int16_t)*PLOT_WIDTH );// 受信したメッセージをバッファへ格納
}

// M5StackPlotクラス
class M5ROS2_Plot : public ros2::Node                 // ros2::Node機能を持ったクラスを作成
{
public:
    M5ROS2_Plot()                                     // コンストラクタ
    : Node("m5stack_plot"), zoom_min_(0), zoom_max_(8)// 初期化時にノード名を設定する
    {
        zoom_ = zoom_min_;
        // Subscriber作成
        this->createSubscriber<m5stack_msgs::Plot>("plot", (ros2::CallbackFunc)subscribePlot, plot_buff_);
                                      // plotトピックをSubscribe, バッファアドレスを渡す
        // Plotデータを初期化
        for(int ii=0;ii<PLOT_WIDTH;++ii){
            plot_buff_[ii] = 0;
        }
    }
    void zoomIn()                                     // ズームイン
    {
        if( zoom_ < zoom_max_ ){
            ++zoom_;
        }
    }
    void zoomOut()                                    // ズームアウト
    {
        if( zoom_ > zoom_min_ ){
            --zoom_;
        }
    }
    void drawPlot()                                   // 描画処理
    {
        uint8_t coef = (zoom_max_ - zoom_);           // ズーム係数作成
        uint8_t pixel_y;

        // データを画面幅全体にプロットする
        for( int ii=0 ; ii<PLOT_WIDTH ; ++ii ){
            int32_t data = plot_buff_[ii];            // データ取り出し 
            data = (data >> coef);                    // ズーム処理
            if( data >= PLOT_MAX ){                   // 最大値制限
                data = PLOT_MAX;
            }else if( data <= PLOT_MIN ){             // 最小値制限
                data = PLOT_MIN;
            }
            data += PLOT_CENTER;                      // 画面座標へオフセット
            pixel_y = (PLOT_HEIGHT - data);           // 上下反転
            M5.Lcd.drawPixel( ii, draw_log_[ii], BLACK );// 前回データでまず消去
            M5.Lcd.drawPixel( ii, pixel_y, WHITE );   // 今回のデータを描画
            draw_log_[ii] = pixel_y;                  // 描画したデータを記録
        }
        M5.Lcd.drawFastHLine( 0, PLOT_CENTER, PLOT_WIDTH, WHITE );// 画面中心線を描画
    }
private:
    int16_t       plot_buff_[PLOT_WIDTH];
    uint8_t       zoom_;
    const uint8_t zoom_min_, zoom_max_;
    uint8_t       draw_log_[PLOT_WIDTH];
};

// Arduinoのセットアップ関数
void setup() {
    M5.begin();                                       // M5Stack機能の初期化
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextColor(WHITE, BLACK);
    WiFi.begin(SSID, SSID_PW);        // WiFi通信の開始（接続先SSID、接続先SSIDのパスワード）
    M5.Lcd.println("Waiting for connection...");
    while(WiFi.status() != WL_CONNECTED);             // WiFiが通信開始するまで待つ
    M5.Lcd.println("  WiFi OK.");
    ros2::init(&udp, AGENT_IP, AGENT_PORT, false);
                                      // ROS2機能の初期化（通信先UDP、AGENTのIPアドレスとポート、TCP無効）
}

// Arduinoのメインループ関数
void loop() {
    static bool ros2_regist = false;
    static M5ROS2_Plot plot_node;                     // Plotノードクラスをstaticで作成
    M5.update();
    if( plot_node.getNodeRegisteredState() ){         // ROS2接続できているか
        if( !ros2_regist ){                           // 接続後初回のみ
            M5.Lcd.println("  ROS2 OK.");             // ROS2の接続状態を表示
            ros2_regist = true;
            delay(500);
            M5.Lcd.fillRect(0,0,PLOT_WIDTH,PLOT_HEIGHT,BLACK);
        }
    }
    if( M5.BtnA.wasPressed() ){                       // ボタンAでズームアウト
        plot_node.zoomOut();
    }
    if( M5.BtnC.wasPressed() ){                       // ボタンCでズームイン
        plot_node.zoomIn();
    }
    if( !M5.BtnB.isPressed() ){                       // ボタンBで一時停止
        plot_node.drawPlot();                         // 押されていなければ描画更新
    }
    ros2::spin(&plot_node);           // Plotノードクラスのspin処理を毎回実行
}
