#include "dotstar.h"
static const char *TAG = "DOTSTAR";
spi_device_handle_t spi;
spi_bus_config_t led_data_bus_cfg={
    .miso_io_num=-1,
    .mosi_io_num=-1,
    .sclk_io_num=-1,
    .quadwp_io_num=-1,	//not used
    .quadhd_io_num=-1,	//not used
    .max_transfer_sz=1
    };
spi_device_interface_config_t adc_spi_dev_cfg={
    .clock_speed_hz=20*1000*1000,           //Clock out at 8 MHz
    .mode=0,                                //SPI mode 0
    .spics_io_num=-1,               //CS pin
    .queue_size=1
    };

static uint8_t m_num_of_leds=0;
static uint8_t m_color_mode=0;
static uint8_t* data_buf;


spi_transaction_t t;

/**
 * @brief Set the Clock Frequency
 * 
 * @param freq 
 */
void setClockFrequency(int freq){
  adc_spi_dev_cfg.clock_speed_hz=freq;
}

/**
 * @brief Setup and Initialise pins and buffers 
 * 
 * @param data_pin 
 * @param clock_pin 
 * @param num_of_leds 
 * @param color_mode - RGB, BGR or GRB
 */
void init_led(uint8_t data_pin, uint8_t clock_pin, uint8_t num_of_leds, uint8_t color_mode){
    led_data_bus_cfg.mosi_io_num=data_pin;
    led_data_bus_cfg.sclk_io_num=clock_pin;
    m_num_of_leds=num_of_leds;
    m_color_mode=color_mode;
    free(data_buf);
    data_buf=heap_caps_malloc(num_of_leds*3,MALLOC_CAP_8BIT);
    memset( data_buf , 0 , num_of_leds*3  );
    esp_err_t ret;
    ret=spi_bus_initialize(HSPI_HOST, &led_data_bus_cfg, 0);
    ESP_ERROR_CHECK(ret);
    ret=spi_bus_add_device(HSPI_HOST, &adc_spi_dev_cfg, &spi);
    ESP_ERROR_CHECK(ret);
    
}

/**
 * @brief Set the Pixel Color(24bit)
 * 
 * @param led_index 
 * @param color 
 * @return int 
 */
int setPixel24bitColor(uint8_t led_index, uint32_t color){
  uint8_t r=0xFF&(color>>16);
  uint8_t g=0xFF&(color>>8);
  uint8_t b=0xFF&(color);
  return setPixelColor(led_index,r,g,b);
}

/**
 * @brief Set the Pixel Color as r g b
 * 
 * @param led_index 
 * @param r 
 * @param g 
 * @param b 
 * @return int 
 */
int setPixelColor(uint8_t led_index,uint8_t r,uint8_t g,uint8_t b){
    if(led_index>m_num_of_leds) {
      ESP_LOGI(TAG,"led index larger than %d",m_num_of_leds);
      return -1;
    }
    uint8_t v0=0, v1=0, v2=0;
    if (m_color_mode == DOTSTAR_RGB) {
      v0 = r; v1 = g; v2 = b;
    } else if (m_color_mode == DOTSTAR_GRB) {
      v0 = g; v1 = r; v2 = b;
    } else if (m_color_mode == DOTSTAR_BGR) {
      v0 = b; v1 = g; v2 = r;
    } 
    data_buf[led_index * 3] = v0;
    data_buf[(led_index * 3) + 1] = v1;
    data_buf[(led_index * 3) + 2] = v2;
    return ESP_OK;
}

/**
 * @brief Display the colors on the LEDs
 * 
 * @return int 
 */
int printLED(){
  esp_err_t ret;
  uint8_t tempBuf[4];
  memset(tempBuf, 0, sizeof(tempBuf));
  memset(&t, 0, sizeof(t));       
  t.length=4*8;                 
  t.tx_buffer=tempBuf;               
  ret=spi_device_transmit(spi, &t);  //Transmit Starting Frame
  assert(ret==ESP_OK);            
  for(uint8_t i =0;i<(m_num_of_leds*3);i+=3){
    tempBuf[0] = 255;
    tempBuf[1] = data_buf[i];
    tempBuf[2] = data_buf[i + 1];
    tempBuf[3] = data_buf[i + 2];
    ret=spi_device_transmit(spi, &t);  //Transmit! LED Frames
    assert(ret==ESP_OK);            
  }
  memset(tempBuf, 0xFF, sizeof(tempBuf));
  ret=spi_device_transmit(spi, &t);  //Transmit END Frame
  assert(ret==ESP_OK);
  return ret;
}