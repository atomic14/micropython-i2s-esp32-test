#include <Arduino.h>
#include <FreeRTOS.h>
#include <SPIFFS.h>
#include "driver/i2s.h"

static const auto RECORD_PIN = GPIO_NUM_23;

// i2s microphone pins
static const i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = GPIO_NUM_26,
    .ws_io_num = GPIO_NUM_22,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = GPIO_NUM_21};

// i2s config for reading from I2S
static const i2s_config_t i2s_mic_Config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s speaker pins
static const i2s_pin_config_t i2s_speaker_pins = {
    .bck_io_num = GPIO_NUM_19,
    .ws_io_num = GPIO_NUM_27,
    .data_out_num = GPIO_NUM_18,
    .data_in_num = I2S_PIN_NO_CHANGE};

// i2s config for speaker output - note this only outputs on the left channel
static const i2s_config_t i2s_speaker_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0};

void setup()
{
  Serial.begin(115200);
  // start up SPIFFS
  SPIFFS.begin(true);

  // set up the button
  pinMode(RECORD_PIN, INPUT_PULLDOWN);

  // start the i2s input

  //install and start i2s driver
  i2s_driver_install(I2S_NUM_0, &i2s_mic_Config, 0, NULL);
  // start the i2s input
  i2s_set_pin(I2S_NUM_0, &i2s_mic_pins);
  // clear the DMA buffers
  i2s_zero_dma_buffer(I2S_NUM_0);
  // and start
  i2s_start(I2S_NUM_0);

  // // start the i2s output

  //install and start i2s driver
  i2s_driver_install(I2S_NUM_1, &i2s_speaker_config, 0, NULL);
  // set up the i2s pins
  i2s_set_pin(I2S_NUM_1, &i2s_speaker_pins);
  // clear the DMA buffers
  i2s_zero_dma_buffer(I2S_NUM_1);
  // and start
  i2s_start(I2S_NUM_1);
}

void wait_for_button()
{
  while (digitalRead(RECORD_PIN) == 0)
  {
    vTaskDelay(100);
  }
  vTaskDelay(100);
}

static int16_t samples[1024];
void loop()
{
  Serial.println("Press and hold button to record");

  wait_for_button();

  Serial.println("Recording");

  FILE *fp_write = fopen("/spiffs/test.raw", "wb");
  while (digitalRead(RECORD_PIN))
  {
    size_t bytes_read = 0;
    i2s_read(I2S_NUM_0, samples, 2048, &bytes_read, portMAX_DELAY);
    // amplify the signal
    for (int i = 0; i < bytes_read / 2; i++)
    {
      samples[i] = samples[i] << 4;
    }
    // save to file
    fwrite(samples, 1, bytes_read, fp_write);
  }
  fclose(fp_write);

  Serial.println("Finished Recording");

  Serial.println("Press the button to playback");

  wait_for_button();

  FILE *fp_read = fopen("/spiffs/test.raw", "rb");
  size_t read_bytes = fread(samples, 1, 2048, fp_read);
  while (read_bytes > 0)
  {
    size_t bytes_written = 0;
    i2s_write(I2S_NUM_1, samples, read_bytes, &bytes_written, portMAX_DELAY);
    read_bytes = fread(samples, 1, 2048, fp_read);
  }
  fclose(fp_read);

  Serial.println("Finished playback");
}