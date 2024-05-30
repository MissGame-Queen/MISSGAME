// Includes
#include "driver/i2s.h" // 适用于ESP32的I2S库
#include "WavData.h"    // 把WAV格式的录音数据的bytes保存到WavData中
#include <Arduino.h>
#include <SD.h>
//  Global Variables/objects
static const i2s_port_t i2s_num = I2S_NUM_1; // i2s port number，注意，如果是用内部DAC必须用I2S_NUM_0
unsigned const char *TheData;
uint32_t DataIdx = 0; // index offset into "TheData" for current  data t send to I2S
// char buffALL[100000];
struct WavHeader_Struct
{
  //   RIFF Section
  char RIFFSectionID[4]; // Letters "RIFF"
  uint32_t Size;         // Size of entire file less 8
  char RiffFormat[4];    // Letters "WAVE"

  //   Format Section
  char FormatSectionID[4]; // letters "fmt"
  uint32_t FormatSize;     // Size of format section less 8
  uint16_t FormatID;       // 1=uncompressed PCM
  uint16_t NumChannels;    // 1=mono,2=stereo
  uint32_t SampleRate;     // 44100, 16000, 8000 etc.
  uint32_t ByteRate;       // =SampleRate * Channels * (BitsPerSample/8)
  uint16_t BlockAlign;     // =Channels * (BitsPerSample/8)
  uint16_t BitsPerSample;  // 8,16,24 or 32

  // Data Section
  char DataSectionID[4]; // The letters "data"
  uint32_t DataSize;     // Size of the data that follows
} WavHeader;

//------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------
// I2S configuration structures

static const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100, // 设置采样率，但由于预先不知道WavData的录音数据的采样率，后面解码后会通过i2s_set_sample_rates修改
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    /*!< standard max*/
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
    .dma_buf_count = 8,                       // 8 buffers
    .dma_buf_len = 1024,                      // 1K per buffer, so 8K of buffer space
    .use_apll = 0,
    .tx_desc_auto_clear = true,
    .fixed_mclk = -1};

// These are the physical wiring connections to our I2S decoder board/chip from the esp32, there are other connections
// required for the chips mentioned at the top (but not to the ESP32), please visit the page mentioned at the top for
// further information regarding these other connections.

static const i2s_pin_config_t pin_config = {
    .bck_io_num = 27,                // 时钟口，对应于MAX38357A的BCLK
    .ws_io_num = 25,                 // 用于声道选择，对应于MAX38357A的LRC
    .data_out_num = 26,              // ESP32的音频输出口, 对应于MAX38357A的DIN
    .data_in_num = I2S_PIN_NO_CHANGE // ESP32的音频输入接口，本例未用到
};
// Audio::setPinout(uint8_t BCLK, uint8_t LRC, uint8_t DOUT, int8_t MCLK)
// audio->setPinout(13, 14, 12);
//  audio->setPinout(27, 25, 26);
//------------------------------------------------------------------------------------------------------------------------
File file;
void PrintData(const char *Data, uint8_t NumBytes)
{
  for (uint8_t i = 0; i < NumBytes; i++)
    Serial.print(Data[i]);
  Serial.println();
}

void DumpWAVHeader(WavHeader_Struct *Wav)
{
  if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
  {
    Serial.print("Not a RIFF format file - ");
    PrintData(Wav->RIFFSectionID, 4);
    return;
  }
  if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
  {
    Serial.print("Not a WAVE file - ");
    PrintData(Wav->RiffFormat, 4);
    return;
  }
  if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
  {
    Serial.print("fmt ID not present - ");
    PrintData(Wav->FormatSectionID, 3);
    return;
  }
  if (memcmp(Wav->DataSectionID, "data", 4) != 0)
  {
    Serial.print("data ID not present - ");
    PrintData(Wav->DataSectionID, 4);
    return;
  }
  // All looks good, dump the data
  Serial.print("Total size :");
  Serial.println(Wav->Size);
  Serial.print("Format section size :");
  Serial.println(Wav->FormatSize);
  Serial.print("Wave format :");
  Serial.println(Wav->FormatID);
  Serial.print("Channels :");
  Serial.println(Wav->NumChannels);
  Serial.print("Sample Rate :");
  Serial.println(Wav->SampleRate);
  Serial.print("Byte Rate :");
  Serial.println(Wav->ByteRate);
  Serial.print("Block Align :");
  Serial.println(Wav->BlockAlign);
  Serial.print("Bits Per Sample :");
  Serial.println(Wav->BitsPerSample);
  Serial.print("Data Size :");
  Serial.println(Wav->DataSize);
}

bool ValidWavData(WavHeader_Struct *Wav)
{

  if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
  {
    Serial.print("Invlaid data - Not RIFF format");
    return false;
  }
  if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
  {
    Serial.print("Invlaid data - Not Wave file");
    return false;
  }
  if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
  {
    Serial.print("Invlaid data - No format section found");
    return false;
  }
  if (memcmp(Wav->DataSectionID, "data", 4) != 0)
  {
    Serial.print("Invlaid data - data section not found");
    return false;
  }
  if (Wav->FormatID != 1)
  {
    Serial.print("Invlaid data - format Id must be 1");
    return false;
  }
  if (Wav->FormatSize != 16)
  {
    Serial.print("Invlaid data - format section size must be 16.");
    return false;
  }
  if ((Wav->NumChannels != 1) & (Wav->NumChannels != 2))
  {
    Serial.print("Invlaid data - only mono or stereo permitted.");
    return false;
  }
  if (Wav->SampleRate > 48000)
  {
    Serial.print("Invlaid data - Sample rate cannot be greater than 48000");
    return false;
  }
  if ((Wav->BitsPerSample != 8) & (Wav->BitsPerSample != 16))
  {
    Serial.print("Invlaid data - Only 8 or 16 bits per sample permitted.");
    return false;
  }
  return true;
}

void setup()
{
  Serial.begin(115200);
  if (!SD.begin(5, SPI, 80000000))
  {
    Serial.println("SD卡發生錯誤");
  }
  else
  {
    Serial.println("SD卡正常!!");
    File root = SD.open("/");
    File file = root.openNextFile();
    Serial.print("檔名:");
    while (file)
    {
      Serial.printf("%s [%d]\n", file.path(), file.size());
      file.close();
      file = root.openNextFile();
    }
    root.close();
  }

  file = SD.open("/0001.wav");
  if (file)

  {
    for (size_t i = 0; i < 78; i++)
    {
      /* code */
      // Serial.print((char)file.read());
    }
  }
  else
    while (1)
      ;
  char buff[78];

  file.readBytes(buff, 78);
  /*
  for (size_t i = 70; i < 78; i++)
  {
    char c = buff[i];

    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
      Serial.print(buff[i]);
    else
      Serial.printf(",%d", (uint8_t)c);
  }
  */
  // memcpy(&WavHeader, &buff, 44); // Copy the header part of the wav data into our structure
  //  Serial.println(WavHeader.DataSectionID);

  // DumpWAVHeader(&WavHeader); // Dump the header data to serial, optional!
  /*
  if (ValidWavData(&WavHeader))
  {
    i2s_driver_install(i2s_num, &i2s_config, 0, NULL); // ESP32 will allocated resources to run I2S
    i2s_set_pin(i2s_num, &pin_config);                 // Tell it the pins you will be using
    i2s_set_sample_rates(i2s_num, 44100);              // set sample rate
  }
  else // end code here
    while (true)
      ;
      */
  i2s_driver_install(i2s_num, &i2s_config, 0, NULL); // ESP32 will allocated resources to run I2S
  i2s_set_pin(i2s_num, &pin_config);                 // Tell it the pins you will be using
  i2s_set_sample_rates(i2s_num, 44100);              // set sample rate
  file.close();
  file = SD.open("/0001.wav");
  // file.readBytes(buffALL, sizeof(buffALL));
  DataIdx = 78;
  file.seek(DataIdx);
  Serial.println("STAR");
  //  WavHeader.DataSize = sizeof(buffALL);
  WavHeader.DataSize = 286858;
}

void loop()
{

  size_t BytesWritten; // Returned by the I2S write routine, we are not interested in it

  char buff[4];
  for (byte i = 0; i < 4; i++)
  //buff[i] = file.read();

  // 这里可选择每次发32bit的数据，也就是4 bytes
  i2s_write(i2s_num, WavData + DataIdx, 4, &BytesWritten, portMAX_DELAY);
  DataIdx += 4;                      // increase the data index to next two 16 bit values (4 bytes)
  if (DataIdx >= WavHeader.DataSize) // If we gone past end of data reset back to beginning
  {
    DataIdx = 78;
    file.seek(DataIdx);
    Serial.println("播放結束!");
  }
}