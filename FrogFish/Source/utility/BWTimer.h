#pragma once

class BWTimer {

private:

    int fps;
    int frames_left = 0;
    int store_frames_left = 0;
    bool started = false;
    bool auto_restart = false;
    void (*callback)() = nullptr;

public:

    BWTimer(int _fps=24) {
        fps = _fps;
    }

    void start(int seconds, int frames, bool _auto_restart=false) {
        started = true;
        frames_left = seconds * fps + frames;
        store_frames_left = frames_left;
        callback = nullptr;
        auto_restart = _auto_restart;
    }

    void start_callback(void (*func)(), int seconds, int frames, bool _auto_restart=false) {
        started = true;
        frames_left = seconds * fps + frames;
        store_frames_left = frames_left;
        callback = func;
        auto_restart = _auto_restart;
    }

    // call once per frame
    void on_frame_update() {
        if (started) {
            --frames_left;
            if (frames_left <= 0) {
                if (callback != nullptr) {
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
};
