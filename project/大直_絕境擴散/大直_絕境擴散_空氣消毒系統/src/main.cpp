#include "main.h"
// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  uint8_t testBypass = 0;

  xTaskCreatePinnedToCore(taskMCP230x7,
                          "taskMCP230x7",
                          2048,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
  
xTaskCreatePinnedToCore(task,
  "task",
  4096,
  (void *)&testBypass,
  1,
  NULL,
  0);

  taskPCM5102((void *)&testBypass);
}
void loop()
{
}
