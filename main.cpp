// Compile and flash with:  mbed compile -f
// Target: K64F
// Toolchain: GCC_ARM

#include "mbed.h"

// Display related:
#include "epd.h"
#include "screenmanager.h"

// Wifi related:
#include "Sender.h"

// SPI
#define SPI_MOSI                                D11
#define SPI_MISO                                D12
#define SPI_SCLK                                D13

// E-PAPER
#define EPD_WIDTH                               400
#define EPD_HEIGHT                              300
#define EPD_CS                                  D9
#define EPD_RESET                               A2
#define EPD_DC                                  D5
#define EPD_BUSY                                D3



SPI spi(SPI_MOSI, SPI_MISO, SPI_SCLK); // mosi, miso, sclk
Epd epd(&spi, EPD_CS, EPD_RESET, EPD_DC, EPD_BUSY); // spi, cs, reset, dc, busy
ScreenManager screen_manager(&epd); // epd
Sender sender;


int main() {
  while(true) {

    // sender.init();

    // static const char* JSON_STRING = sender.get("172.16.10.155", "02.85");
    static const char* JSON_STRING = "{\"currentTime\":\"09:03:00\",\"roomDescription\":\"Labo Mbedded ontwerp\",\"roomName\":\"02.85\",\"events\":[{\"teacher\":\"Diddl en Bob de Bouwer\",\"start\":\"09:30\",\"stop\":\"11:30\",\"date\":\"2019-04-04\",\"coursename\":\"Kookles\",\"description\":\"Kunnen we het maken? Nou en of!\"},{\"teacher\":\"Dora the explorer\",\"start\":\"11:30\",\"stop\":\"13:30\",\"date\":\"2019-04-04\",\"coursename\":\"Aardrijkskunde\",\"description\":\"We did it! We did it! We did it!\"},{\"teacher\":\"Espeel Ludovic, Lannoo Jonas\",\"start\":\"13:30\",\"stop\":\"15:30\",\"date\":\"2019-04-04\",\"coursename\":\"Projectlab bachelor elektronica-ICT\",\"description\":\"B-KUL-B3390N\"},{\"teacher\":\"meneer Boma\",\"start\":\"15:45\",\"stop\":\"17:45\",\"date\":\"2019-04-04\",\"coursename\":\"Chemie\",\"description\":\"Mijn gedacht...\"}]}";
    
    printf("\n\r%s\n\r\n\r", JSON_STRING);

    screen_manager.reservationScreen(JSON_STRING);

  
    ThisThread::sleep_for(300000);  // refresh every 5 min
  }
}