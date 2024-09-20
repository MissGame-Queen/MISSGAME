// 觀落茵_廚房 2020.1.30
// in Miss Game
// jacky lin 修正
// 繼電器
// 鍋蓋 NO1 START NC2 END NO3 櫃子 null 火1 NO5 火2 NO6 火3 NO7 火4 NO8

#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <HX711.h>

#define reset_btn 2        // reset鍵(i)     原2
//#define reset_light 3      // reset指示燈(o)  原3
#define mp3_rx A5          // MP3模組 rx      原4
#define mp3_tx A4          // MP3模組 tx      原5
#define scale_DOUT A0      // HX711     原6
#define scale_SCK A1       // HX711     原7
#define pot_cover 4        // 鍋蓋 磁力鎖(o)    原8
#define wind_sensor 5      // 搧風感應麥克風(i)  原9
#define fish_lock 6        // 未煮 磁力鎖(o)  原10
#define gate_sensor 3      // 開門微動感測(i)  原11
#define pot_cover_sensor 8 // 鍋蓋 磁簧(i)    原12
#define fish_finish 9      // 煮完 磁力鎖(o)  原13

#define btn_D0 A2 // D0遙控器(i)   原19(A5)
#define btn_D1 A3 // D1遙控器(i)   原18(A4)

#define fire_1 10 // 火力燈1(o)      原14(A0)
#define fire_2 11 // 火力燈2(o)      原15(A1)
#define fire_3 12 // 火力燈3(o)      原16(A2)
#define fire_4 13 // 火力燈4(o)      原17(A3)

#define int_mp3_volume 10
#define wind_betwe en 20000 // 10秒內
#define wind_time 50        // 搧X次
#define wind_delay 3000     // 幾秒數一次
#define fish_change_time 5000

#define scale_basic 385 // 校準值  315
#define fish_weight 480 // 單位g 20200213 jacky Lin校整
#define deviation 20    // 誤差g +-5g 2020.4.20 修改 +-20

int now_game_state = 0xFF, game_state = 0;
bool ifMP3OK = true;
bool ifFast = false;

HX711 scale;
SoftwareSerial mySoftwareSerial(mp3_rx, mp3_tx); // RX, TX
#define mp3_Serial mySoftwareSerial
DFRobotDFPlayerMini Mp3Player_1; // mp3模組

enum Status_e
{
    _Reset,        // RE
    _WaitDoorOpen, // 等門開
    _HX711Reading, // 稱重
    _CloseTheLid,  // 蓋上蓋子
    _HuaFeng,      // 搧風
    _OpenTheLid,   // 打開蓋子
    _Finish,
    _DEBUG,
};
void initialize_MP3()
{
    mp3_Serial.begin(9600);
    Serial.println(F("Initializing DFPlayer ..."));
    if (!Mp3Player_1.begin(mp3_Serial, /*isACK = */ true, /*doReset = */ true))
    {
        Serial.println(F("mp3_1 Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
        ifMP3OK = false;
    }
    else
    {
        Serial.println(F("DFPlayer Mini online."));
        ifMP3OK = true;
    }
    if (ifMP3OK)
    {
        Mp3Player_1.setTimeOut(500);
        Mp3Player_1.volume(int_mp3_volume);
        Mp3Player_1.outputDevice(DFPLAYER_DEVICE_SD);
        Mp3Player_1.enableDAC();
        Mp3Player_1.enableLoop();
    }
}

void initialize_HX711()
{
    Serial.println("HX711 Demo");
    Serial.println("Initializing the scale");

    scale.begin(scale_DOUT, scale_SCK);
    delay(1000);
    scale.set_scale(scale_basic);
    if (scale.is_ready())
    {
        long reading = scale.get_units(10);
        scale.tare();
        Serial.print("HX711 reading: ");
        Serial.println(reading);
    }
    else
    {
        Serial.println("HX711 not found.");
    }
}

void initialize_pinMode()
{
    pinMode(reset_btn, INPUT_PULLUP);
    pinMode(pot_cover_sensor, INPUT_PULLUP);
    pinMode(gate_sensor, INPUT_PULLUP);

    pinMode(wind_sensor, INPUT);
    pinMode(btn_D0, INPUT);
    pinMode(btn_D1, INPUT);

    //pinMode(reset_light, OUTPUT);
    pinMode(pot_cover, OUTPUT);
    pinMode(fish_lock, OUTPUT);
    pinMode(fish_finish, OUTPUT);
    pinMode(fire_1, OUTPUT);
    pinMode(fire_2, OUTPUT);
    pinMode(fire_3, OUTPUT);
    pinMode(fire_4, OUTPUT);

    //digitalWrite(reset_light, LOW);
    digitalWrite(pot_cover, LOW);
    digitalWrite(fish_lock, LOW);
    digitalWrite(fish_finish, LOW);
    digitalWrite(fire_1, LOW);
    digitalWrite(fire_2, LOW);
    digitalWrite(fire_3, LOW);
    digitalWrite(fire_4, LOW);
}

void setup()
{

    Serial.begin(115200);
    initialize_pinMode();
    initialize_MP3();
    initialize_HX711();

    Serial.println("Run~~~~~");
}

void loop()
{
    // 讀RE按鈕、遙控器RE按鈕
    if (digitalRead(reset_btn) == LOW || digitalRead(btn_D1) == HIGH)
    {
        game_state = _Reset;
        Serial.println("手動RE!");
        delay(1000);
    }
    // 讀取跳關遙控器按鈕
    if (digitalRead(btn_D0) == HIGH)
    {
        Serial.println("跳至下一步 >>");
        game_state = game_state + 1;
        ifFast = true;
        delay(1000);
    }
    if (now_game_state != game_state)
    { // 顯示狀態更新
        Serial.print("game_state : ");
        Serial.println(game_state);
        now_game_state = game_state;
    }

    if (game_state != 0)
    {
        //digitalWrite(reset_light, HIGH);
    }
    switch (game_state)
    {
    case _Reset:
    {
        //digitalWrite(reset_light, LOW); // reset燈亮
        digitalWrite(pot_cover, LOW);   // 鍋蓋磁力鎖
        digitalWrite(fish_lock, LOW);   // 未煮 磁力鎖
        digitalWrite(fish_finish, LOW); // 煮完 磁力鎖
        digitalWrite(fire_1, LOW);
        digitalWrite(fire_2, LOW);
        digitalWrite(fire_3, LOW);
        digitalWrite(fire_4, LOW);
        if (ifMP3OK)
            Mp3Player_1.pause();
        initialize_HX711();
        Serial.println("RE!");
        delay(500);
        ifFast = true;
        game_state++;
    }
    break;
    case _WaitDoorOpen:
    {
        if (ifFast)
        {
            Serial.println("等待門打開");
            ifFast = false;
        }
        if (digitalRead(gate_sensor) == LOW)
        {
            Serial.println("門被打開!播放背景音樂");

            if (ifMP3OK)
            {
                Mp3Player_1.volume(int_mp3_volume);
                delay(20);
                Mp3Player_1.play(1);
                delay(20);
            }
            game_state++;
            ifFast = true;
            delay(100);
        }
    }
    break;
    case _HX711Reading:
    {
        static long old_reading = 0, reading = 0;
        if (ifFast)
        {
            Serial.println("開始稱重");
            ifFast = false;
            reading = 0;
            old_reading = 0;
        }
        reading = scale.get_units(1);
        if (reading != old_reading)
        {                                    // 20200212 jacky Lin
            Serial.print("HX711 reading: "); // 顯示重量更新
            Serial.println(reading);
            old_reading = reading;
        }
        if (check_weight(reading, fish_weight, deviation))
        { // 重量檢測是否符合
            delay(2000);
            if (check_weight(reading, fish_weight, deviation))
            {
                game_state++;
                ifFast = true;
            }
        }
    }
    break;
    case _CloseTheLid:
    {
        if (ifFast)
        {
            //digitalWrite(reset_light, HIGH); // reset燈亮
            Serial.println("等待關上蓋子#BYPASS待維修");
            ifFast = false;
            scale.power_down();
            delay(20);
        }
        // 讀鍋蓋磁簧
        if (digitalRead(pot_cover_sensor) == LOW)
        {
            if (ifMP3OK)
            {
                Mp3Player_1.volume(1);
                delay(20);
                Mp3Player_1.play(2);
                delay(50);
            }
            game_state++;
            ifFast = true;
        }
    }
    break;
    case _HuaFeng:
    {
        static uint32_t timer = 0;
        static int wind_state = 0; // 紀錄連續搧風幾次
        if (ifFast)
        {
            Serial.println("執行搧風");
            ifFast = false;
            digitalWrite(pot_cover, HIGH); // 鍋蓋上鎖
            digitalWrite(fire_1, HIGH);    // 火力1
            delay(50);
            timer = millis();
            wind_state = 0;
        }
        // 超過時間沒搧風
        if (millis() > timer + wind_delay)
        {
            wind_state = 0;
            Serial.print("wind_state : ");
            Serial.println(wind_state);
            if (ifMP3OK)
                Mp3Player_1.volume(5);
            digitalWrite(fire_2, LOW);
            digitalWrite(fire_3, LOW);
            timer = millis();
        }
        bool windread = digitalRead(wind_sensor);
        delay(100);
        if (windread == HIGH)
        {
            wind_state++;
            Serial.print("wind_state : ");
            Serial.println(wind_state);
            timer = millis();
        }

        if (wind_state > wind_time / 3 && wind_state <= wind_time * 2 / 3)
            digitalWrite(fire_2, HIGH);
        else if (wind_state > wind_time * 2 / 3 && wind_state < wind_time)
            digitalWrite(fire_3, HIGH);
        else if (wind_state >= wind_time)
        {
            digitalWrite(fire_4, HIGH);
            digitalWrite(fish_lock, HIGH);
            digitalWrite(fish_finish, HIGH);

            for (int y = 15; y < 30; y++)
            {
                Mp3Player_1.volume(y);
                Serial.print("fire more and more...");
                Serial.println(y);
                delay(1000);
            }

            game_state++;
        }
    }
    break;
    case _OpenTheLid:
    {
        Serial.println("執行開蓋");

        digitalWrite(fire_2, HIGH);
        digitalWrite(fire_3, HIGH);
        digitalWrite(fire_4, HIGH);
        digitalWrite(fish_lock, HIGH);
        digitalWrite(fish_finish, HIGH);
        if (ifMP3OK)
            Mp3Player_1.volume(30);
        digitalWrite(pot_cover, LOW);
        game_state++;
    }
    break;
    case _Finish:
    {

        digitalWrite(fish_finish, HIGH);
        digitalWrite(pot_cover, LOW);
        // digitalWrite(fire_2,LOW);
        // digitalWrite(fire_3,LOW);
        // digitalWrite(fire_4,LOW);
        if (ifMP3OK)
        {
            for (int y = 30; y > 0; y--)
            {
                Mp3Player_1.volume(y);
                delay(50);
            }
            Mp3Player_1.play(3);
        }
        delay(5);
        game_state++;
    }
    break;
    }
    if (Mp3Player_1.available())
    {
        printDetail(Mp3Player_1.readType(), Mp3Player_1.read()); // Print the detail message from DFPlayer to handle different errors and states.
    }
    delay(50); // 避免Serial跟不上
}

boolean check_weight(unsigned int now_weight, unsigned int target_weight, unsigned int allow_deviation)
{
    unsigned int max_value = target_weight + allow_deviation;
    unsigned int min_value = target_weight - allow_deviation;
    if (now_weight < max_value && now_weight > min_value)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void printDetail(uint8_t type, int value)
{
    switch (type)
    {
    case TimeOut:
        Serial.println(F("Time Out!"));
        break;
    case WrongStack:
        Serial.println(F("Stack Wrong!"));
        break;
    case DFPlayerCardInserted:
        Serial.println(F("Card Inserted!"));
        break;
    case DFPlayerCardRemoved:
        Serial.println(F("Card Removed!"));
        break;
    case DFPlayerCardOnline:
        Serial.println(F("Card Online!"));
        break;
    case DFPlayerUSBInserted:
        Serial.println("USB Inserted!");
        break;
    case DFPlayerUSBRemoved:
        Serial.println("USB Removed!");
        break;
    case DFPlayerPlayFinished:
        Serial.print(F("Number:"));
        Serial.print(value);
        Serial.println(F(" Play Finished!"));
        break;
    case DFPlayerError:
        Serial.print(F("DFPlayerError:"));
        switch (value)
        {
        case Busy:
            Serial.println(F("Card not found"));
            break;
        case Sleeping:
            Serial.println(F("Sleeping"));
            break;
        case SerialWrongStack:
            Serial.println(F("Get Wrong Stack"));
            break;
        case CheckSumNotMatch:
            Serial.println(F("Check Sum Not Match"));
            break;
        case FileIndexOut:
            Serial.println(F("File Index Out of Bound"));
            break;
        case FileMismatch:
            Serial.println(F("Cannot Find File"));
            break;
        case Advertise:
            Serial.println(F("In Advertise"));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}