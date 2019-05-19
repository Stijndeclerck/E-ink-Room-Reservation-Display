#pragma once
#include "epd.h"
#include "paint.h"
#include "MbedJSONValue.h"
#include <string>

// Display
#define COLORED                                 0
#define UNCOLORED                               1

// JSON ID's
#define CURRENT_TIME                        "currentTime"
#define ROOM_NAME_ID                        "roomName"
#define ROOM_DESCRIPTION_ID                 "roomDescription"
#define EVENTS_ID                           "events"
#define SPEAKER_ID                          "teacher"
#define START_TIME_ID                       "start"
#define STOP_TIME_ID                        "stop"
#define DATE_ID                             "date"
#define EVENT_NAME_ID                      "coursename"
#define EVENT_DESCRIPTION_ID               "description"

class ScreenManager
{
    public: 
        ScreenManager(Epd* epd);
        ~ScreenManager();

        void init();

        void reservationScreen(const char* JSON_STRING);

    protected:
        
        Epd* epd;
        int window_width;
        int window_height;
        int chunk_width;
        int chunk_height;

};