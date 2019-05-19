#include "epd.h"

Epd::Epd(SPI* spi, PinName cs_pin, PinName reset_pin, PinName dc_pin, PinName busy_pin):
  cs(cs_pin), display_reset(reset_pin), dc(dc_pin), busy(busy_pin)
{
    this->spi = spi;
    spi->format(8, 3);
    spi->frequency(2000000);
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
    init();
}

Epd::~Epd(){};

void Epd::init()
{
    /* EPD hardware init start */
    reset();
    sendCommand(BOOSTER_SOFT_START);
    sendData(0x17);
    sendData(0x17);
    sendData(0x17);     //07 0f 17 1f 27 2F 37 2f
    sendCommand(POWER_ON);
    waitUntilIdle();
    sendCommand(PANEL_SETTING);
    sendData(0x0F);     // LUT from OTP
    /* EPD hardware init end */
}

int Epd::getWidth()
{
    return this->width;
}

int Epd::getHeight()
{
    return this->height;
}

void Epd::sendCommand(unsigned char command)
{
    dc = 0;
    cs = 0;
    spi->write(command);
    cs = 1;
}

void Epd::sendData(unsigned char data)
{
    dc = 1;
    cs = 0;
    spi->write(data);
    cs = 1;
}

unsigned char Epd::receiveData(void)
{
    unsigned char data;
    dc = 1;
    cs = 0;
    data = spi->write(0x00);
    cs = 1;
    return data;
}

void Epd::waitUntilIdle()
{

    ThisThread::sleep_for(3000);

    // TODO: Fix reading pin or reading eeprom w/ SPI

    ThisThread::sleep_for(1000);

    // sendCommand(GET_STATUS);
    // unsigned char data = receiveData();
    // printf("Status = 0x%X\n\r", data);

    // Pin not working ATM
    // while(!busy){   // 0:busy, 1: idle
    //     ThisThread::sleep_for(100);
    // }
}

void Epd::reset()
{
    display_reset = 0;
    ThisThread::sleep_for(200);
    display_reset = 1;
    ThisThread::sleep_for(200);
}

void Epd::clearFrame()
{
    sendCommand(DATA_START_TRANSMISSION_1);
    ThisThread::sleep_for(2);
    for(int i = 0; i < width / 8 * height; i++) {
        sendData(0xFF);
    }
    ThisThread::sleep_for(2);
    sendCommand(DATA_START_TRANSMISSION_2);
    ThisThread::sleep_for(2);
    for(int i = 0; i < width / 8 * height; i++) {
        sendData(0xFF);
    }
    ThisThread::sleep_for(2);
}

/**
 * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power. 
 *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a 
 *         check code, the command would be executed if check code = 0xA5. 
 *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
 */
void Epd::sleep()
{
    sendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    sendData(0xF7);     // border floating
    sendCommand(POWER_OFF);
    waitUntilIdle();
    sendCommand(DEEP_SLEEP);
    sendData(0x15);
}

void Epd::displayFrame()
{
    sendCommand(DISPLAY_REFRESH);
    ThisThread::sleep_for(100);
    waitUntilIdle();
}

void Epd::displayFrame(const unsigned char* frame_black, const unsigned char* frame_red) 
{
    if (frame_black != NULL) {
        sendCommand(DATA_START_TRANSMISSION_1);
        ThisThread::sleep_for(2);
        for (int i = 0; i < this->width / 8 * this->height; i++) {
            sendData(frame_black[i]);
        }
        ThisThread::sleep_for(2);
    }
    if (frame_red != NULL) {
        sendCommand(DATA_START_TRANSMISSION_2);
        ThisThread::sleep_for(2);
        for (int i = 0; i < this->width / 8 * this->height; i++) {
            sendData(frame_red[i]);
        }
        ThisThread::sleep_for(2);
    }
    sendCommand(DISPLAY_REFRESH);
    waitUntilIdle();
}

/**
 *  @brief: transmit partial data to the black part of SRAM
 */
void Epd::setPartialWindowBlack(const unsigned char* buffer_black, int x, int y, int w, int l) {
    sendCommand(PARTIAL_IN);
    sendCommand(PARTIAL_WINDOW);
    sendData(x >> 8);
    sendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    sendData(((x & 0xf8) + w  - 1) >> 8);
    sendData(((x & 0xf8) + w  - 1) | 0x07);
    sendData(y >> 8);        
    sendData(y & 0xff);
    sendData((y + l - 1) >> 8);        
    sendData((y + l - 1) & 0xff);
    sendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    ThisThread::sleep_for(2);
    sendCommand(DATA_START_TRANSMISSION_1);
    if (buffer_black != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            sendData(buffer_black[i]);  
        }  
    }
    ThisThread::sleep_for(2);
    sendCommand(PARTIAL_OUT);  
}

/**
 *  @brief: transmit partial data to the red part of SRAM
 */
void Epd::setPartialWindowRed(const unsigned char* buffer_red, int x, int y, int w, int l) 
{
    sendCommand(PARTIAL_IN);
    sendCommand(PARTIAL_WINDOW);
    sendData(x >> 8);
    sendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    sendData(((x & 0xf8) + w  - 1) >> 8);
    sendData(((x & 0xf8) + w  - 1) | 0x07);
    sendData(y >> 8);        
    sendData(y & 0xff);
    sendData((y + l - 1) >> 8);        
    sendData((y + l - 1) & 0xff);
    sendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    ThisThread::sleep_for(2);
    sendCommand(DATA_START_TRANSMISSION_2);
    if (buffer_red != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            sendData(buffer_red[i]);
        }  
    }
    ThisThread::sleep_for(2);
    sendCommand(PARTIAL_OUT);  
}