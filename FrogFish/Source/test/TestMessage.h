#pragma once

#include "string.h"
#include "stdio.h"

#define DBGMSG Test::Message::unique_buffered_messages_add

namespace Test::Message {

    const int MAX_MESSAGE_LEN = 500;

    void init(int interval);

    void on_frame_update(); 

    void unique_buffered_messages_add(const char* msg);

    template <typename T>
    void unique_buffered_messages_add(const char* msg, T item) {
        char buff[MAX_MESSAGE_LEN];
        sprintf(buff, msg, item);
        unique_buffered_messages_add(buff);
    }

    template <typename T>
    void unique_buffered_messages_add(const char* msg, T item1, T item2) {
        char buff[MAX_MESSAGE_LEN];
        sprintf(buff, msg, item1, item2);
        unique_buffered_messages_add(buff);
    }
}