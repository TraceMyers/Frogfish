#pragma once

#include "../unitdata/FrogUnit.h"

template <class T>
class BWTimer {

private:

    int fps;
    int frames_left;
    int store_frames_left;
    bool started;
    bool auto_restart;
    void (*callback)();
    void (*obj_callback)(T obj);
    T callback_obj_ptr;

public:

    BWTimer(int _fps=24) :
        fps(_fps),
        frames_left(0),
        store_frames_left(0),
        started(false),
        auto_restart(false),
        callback(nullptr),
        obj_callback(nullptr),
        callback_obj_ptr(nullptr)
    {}

    void start(int seconds, int frames, bool _auto_restart=false) {
        started = true;
        frames_left = seconds * fps + frames;
        store_frames_left = frames_left;
        callback = nullptr;
        obj_callback = nullptr;
        callback_obj_ptr = nullptr;
        auto_restart = _auto_restart;
    }

    void start(void (*func)(), int seconds, int frames, bool _auto_restart=false) {
        started = true;
        frames_left = seconds * fps + frames;
        store_frames_left = frames_left;
        callback = func;
        obj_callback = nullptr;
        callback_obj_ptr = nullptr;
        auto_restart = _auto_restart;
    }

    template <class T>
    void start(
        T obj,
        void (*func)(T param), 
        int seconds, 
        int frames, 
        bool _auto_restart=false
    ) {
        started = true;
        frames_left = seconds * fps + frames;
        store_frames_left = frames_left;
        callback = nullptr;
        obj_callback = func;
        callback_obj_ptr = obj;
        auto_restart = _auto_restart;
    }

    // call once per frame
    void on_frame_update() {
        if (started) {
            --frames_left;
            if (frames_left <= 0) {
                if (callback_obj_ptr != nullptr) {
                    obj_callback(callback_obj_ptr);
                }
                else if (callback != nullptr) {
                    callback();
                }
                if (auto_restart) {
                    frames_left = store_frames_left;
                }
                else {
                    started = false;
                }
            }
        }
    }

    bool is_stopped() {
        return !started;
    }

    void restart() {
        frames_left = store_frames_left;
        started = true;
    }

    int get_frames_left() {
        return frames_left;
    }
};
