#pragma once

#include <android/native_window.h>
#include <atomic>
#include <string>
#include <thread>

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaMuxer.h>

namespace record {

class VideoEncoder {
public:
    VideoEncoder() = default;
    ~VideoEncoder();

    VideoEncoder(const VideoEncoder&) = delete;
    VideoEncoder& operator=(const VideoEncoder&) = delete;

    bool start(
        const std::string& outputPath,
        int width = 2560,
        int height = 1440,
        int fps = 60,
        int bitrate = 35000000
    );

    void stop();

    ANativeWindow* inputSurface() const;
    bool isRunning() const;
    std::string error() const;

private:
    void drainLoop();

    AMediaCodec* m_codec = nullptr;
    AMediaMuxer* m_muxer = nullptr;
    ANativeWindow* m_surface = nullptr;

    std::thread m_thread;

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_stopping{false};
    std::atomic<bool> m_failed{false};

    std::string m_error;

    int m_fd = -1;
    int m_track = -1;

    bool m_muxerStarted = false;
};

}
