#include "TestMessage.h"
#include "../utility/BWTimer.h"
#include "stdio.h"
#include <string>
#include <vector>

namespace Test::Message {

    namespace {

        std::vector<std::string> unique_interval_messages;
        BWTimer unique_interval_message_timer;
        int _interval = 5;

        void unique_buffered_messages_print_all() {
            if (unique_interval_messages.size() > 0) {
                printf("Test::Message::[Unique Buffered Messages]\n---------------------------------------\n");
                for (auto& str : unique_interval_messages) {
                    printf("%s\n", str.c_str());
                }
                printf("---------------------------------------\n");
                unique_interval_messages.clear();
            }
        }
    }
    
    void init(int interval) {
        unique_interval_message_timer.start(interval, 0);
    }

    void on_frame_update() {
        unique_interval_message_timer.on_frame_update();
        if (unique_interval_message_timer.is_stopped()) {
            unique_buffered_messages_print_all();
            unique_interval_message_timer.restart();
        }
    }

    void unique_buffered_messages_add(const char* msg) {
        bool already_buffered = false;
        for (auto& str : unique_interval_messages) {
            if (strcmp(str.c_str(), msg) == 0) {
                already_buffered = true;
                break;
            }
        }
        if (!already_buffered) {
            unique_interval_messages.push_back(std::string(msg));
        }
    }
}