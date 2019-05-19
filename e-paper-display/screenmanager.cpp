#include "screenmanager.h"

ScreenManager::ScreenManager(Epd* epd)
{
    this->epd = epd;
    init();
}

ScreenManager::~ScreenManager(){};

void ScreenManager::init()
{
    this->window_width = epd->getWidth();
    this->window_height = epd->getHeight();

    // TODO: refactor this
    // chunk size defindes how big the partial screen buffer is in RAM, too big of a screnbuffer will result in a stack overflow
    this->chunk_width = this->window_width;
    this->chunk_height = 1;
}

void ScreenManager::reservationScreen(const char* JSON_STRING)
{
    MbedJSONValue json;
    parse(json, JSON_STRING);                                                                   // Parse json string

    epd->init();                                                                                // Initialize e-paper display
    epd->clearFrame();                                                                          // Clear S-RAM on e-paper dispaly

    std::string room_name = json[ROOM_NAME_ID].get<std::string>();                              // Parse rooom name
    std::string room_description = json[ROOM_DESCRIPTION_ID].get<std::string>();                // Parse room description
    std::string current_time = json[CURRENT_TIME].get<std::string>();                           // Parse current time

    int next_event_id = -1;
    int current_event_id = -1;
    for(int i = 0; i < json[EVENTS_ID].size(); i++) {
      std::string start_time = json[EVENTS_ID][i][START_TIME_ID].get<std::string>().substr(0,5);
      std::string stop_time = json[EVENTS_ID][i][STOP_TIME_ID].get<std::string>().substr(0,5);
      std::string event_name = json[EVENTS_ID][i][EVENT_NAME_ID].get<std::string>().substr(0,20);
      std::string speaker = json[EVENTS_ID][i][SPEAKER_ID].get<std::string>().substr(0,35);

      printf("Start time %dh %dm, current time %dh %dm\r\n", stoi(start_time.substr(0,2)), stoi(start_time.substr(3,4)), stoi(current_time.substr(0,2)), stoi(current_time.substr(3,4)));
      printf("Stop time %dh %dm, current time %dh %dm\r\n", stoi(stop_time.substr(0,2)), stoi(stop_time.substr(3,4)), stoi(current_time.substr(0,2)), stoi(current_time.substr(3,4)));

      if(stoi(start_time.substr(0,2)) * 60 + stoi(start_time.substr(3,4)) <= stoi(current_time.substr(0,2)) * 60 + stoi(current_time.substr(3,4))){
          if(stoi(stop_time.substr(0,2)) * 60 + stoi(stop_time.substr(3,4)) >= stoi(current_time.substr(0,2)) * stoi(current_time.substr(3,4))){
              current_event_id = i;
              printf("Current id: %d\n\r", current_event_id);
        }
      }
      if(stoi(start_time.substr(0,2)) * 60 + stoi(start_time.substr(3,4)) > stoi(current_time.substr(0,2)) * 60 + stoi(current_time.substr(3,4))){
          next_event_id = i;
          printf("Next id: %d\n\r", next_event_id);
          break;
      }
    }

    printf("Current id: %d\n\r", current_event_id);
    printf("Next id: %d\n\r", next_event_id);



    unsigned char image[(chunk_height * chunk_width) / 8];                                      // Creates char array that will contain partial screen buffer
    Paint paint(image, chunk_width, chunk_height);                                              // Creates paint object

    // BLACK
    for(int y = 0; y < window_height; y+= paint.GetHeight()) {
      for(int x = 0; x < window_width; x+= paint.GetWidth()) {
        paint.Clear(UNCOLORED);

        // Location
        paint.DrawStringAt(5 - x, 9 - y, room_name.c_str(), &Font24, COLORED);                                                        // Align left
        paint.DrawStringAt(395 - room_description.length() * Font16.Width - x, 14 - y, room_description.c_str(), &Font16, COLORED);   // ALign right
        paint.DrawFilledRectangle(0 - x, 29 - y, 399, 31 - y, COLORED);

        // Timetable
        for(int i = 0; i < 13; i++) {   // 08:00 to 20:00 (12h)
          char time[5];
          sprintf(time, "%02d:00", i + 8);
          paint.DrawHorizontalLine(328 - x, i * 20 - y + 50, 64, COLORED);
          paint.DrawStringAt(290 - x, i * 20 - y + 43, time, &Font12, COLORED);
        }

        // Status
        if(current_event_id >= 0){
          std::string event_name = json[EVENTS_ID][current_event_id][EVENT_NAME_ID].get<std::string>().substr(0,16);
          std::string stop_time = json[EVENTS_ID][current_event_id][STOP_TIME_ID].get<std::string>().substr(0,5);
          std::string speaker = json[EVENTS_ID][current_event_id][SPEAKER_ID].get<std::string>().substr(0,35);
          
          paint.DrawStringAt(10 - x, 50 - y, event_name.c_str(), &Font24, COLORED);
          paint.DrawStringAt(12 - x, 75 - y, speaker.c_str(), &Font16, COLORED);
          char event_end[10];
          sprintf(event_end, "tot %s", stop_time.c_str());
          paint.DrawStringAt(12 - x, 90 - y, event_end, &Font12, COLORED);

        } else {
          paint.DrawStringAt(115 - x, 59 - y, "Vrij", &Font24, COLORED);
          if(next_event_id >= 0){
            std::string start_time = json[EVENTS_ID][next_event_id][START_TIME_ID].get<std::string>().substr(0,5);
            char next_start[10];
            sprintf(next_start, "tot %s", start_time.c_str());
            paint.DrawStringAt(118 - x, 90 - y, next_start, &Font12, COLORED);            
          }
        }


        // Info
        paint.DrawFilledRectangle(0 - x, 120 - y, 280 - x, 121 - y, COLORED); // Thicc line
        for(int i = 0; i < 3; i++) {
          paint.DrawHorizontalLine(0 - x, i * 50 - y + 170, 270, COLORED);    // Norminal lines
        }

        if(next_event_id >= 0){
          for(int i = next_event_id; i < json[EVENTS_ID].size() && i < next_event_id + 4; i++) {
            std::string start_time = json[EVENTS_ID][i][START_TIME_ID].get<std::string>().substr(0,5);
            std::string stop_time = json[EVENTS_ID][i][STOP_TIME_ID].get<std::string>().substr(0,5);
            std::string event_name = json[EVENTS_ID][i][EVENT_NAME_ID].get<std::string>().substr(0,20);
            std::string speaker = json[EVENTS_ID][i][SPEAKER_ID].get<std::string>().substr(0,35);

            int pos = i - next_event_id;

            paint.DrawStringAt(5 - x, pos * 50 + 130 - y, start_time.c_str(), &Font12, COLORED);
            paint.DrawStringAt(5 - x, pos * 50 + 150 - y, stop_time.c_str(), &Font12, COLORED);

            paint.DrawStringAt(50 - x, pos * 50 + 128 - y, event_name.c_str(), &Font16, COLORED);
            paint.DrawStringAt(55 - x, pos * 50 + 149 - y, speaker.c_str(), &Font12, COLORED);
          }
        }
        
        epd->setPartialWindowBlack(paint.GetImage(), x, y, paint.GetWidth(), paint.GetHeight());
      }
    }

    // RED
    for(int y = 0; y < window_height; y+= paint.GetHeight()) {
      for(int x = 0; x < window_width; x+= paint.GetWidth()) {
        paint.Clear(UNCOLORED);

        // Time table
        for(int i = 0; i < json[EVENTS_ID].size(); i++) {
          std::string start_time = json[EVENTS_ID][i][START_TIME_ID].get<std::string>();
          int start_time_h = std::stoi(start_time.substr (0,2));
          int start_time_m = std::stoi(start_time.substr (3,4));
  
          std::string stop_time = json[EVENTS_ID][i][STOP_TIME_ID].get<std::string>();
          int stop_time_h = std::stoi(stop_time.substr (0,2));
          int stop_time_m = std::stoi(stop_time.substr (3,4));

          // printf("%s Start time is %dh and %dm\n\r",start_time.c_str(), start_time_h, start_time_m);
          // printf("%s Stop time is %dh and %dm\n\r",stop_time.c_str(), stop_time_h, stop_time_m);

          int start_y =  50 + (start_time_h - 8) * 20 + (start_time_m / 3);
          int stop_y =  50 + (stop_time_h - 8) * 20 + (stop_time_m / 3);

          paint.DrawFilledRectangle(333 - x, start_y- y, 387 - x, stop_y - y, COLORED);
        }
        for(int i = 0; i < 13; i++) {   // Remove red where there sould be black
          paint.DrawHorizontalLine(328 - x, i * 20 - y + 50, 64, UNCOLORED);
        }

        epd->setPartialWindowRed(paint.GetImage(), x, y, paint.GetWidth(), paint.GetHeight());
      }
    }
    
    // This displays the data from the SRAM in e-Paper module
    epd->displayFrame();

    // Deep sleep
    epd->sleep();
}