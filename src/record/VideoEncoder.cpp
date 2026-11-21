#include "VideoEncoder.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <cstring>

namespace record {

static bool makeDirectories(const std::string& path) {
    std::string current;

    for (char c : path) {
        current += c;

        if (c == '/') {
            if (current.size() > 1)
                mkdir(current.c_str(), 0775);
        }
    }

    return true;
}

VideoEncoder::~VideoEncoder() {
    stop();
}

bool VideoEncoder::start(
    const std::string& outputPath,
    int width,
    int height,
    int fps,
    int bitrate
) {
    if (m_running)
        return false;

    m_error.clear();
    m_failed = false;
    m_stopping = false;
    m_muxerStarted = false;
    m_track = -1;

    if (width <= 0 || height <= 0 || fps <= 0 || bitrate <= 0) {
        m_error = "Invalid encoder settings";
        return false;
    }

    if ((width & 1) || (height & 1)) {
        m_error = "Video dimensions must be even";
        return false;
    }

    auto slash = outputPath.find_last_of('/');
    if (slash != std::string::npos)
        makeDirectories(outputPath.substr(0, slash + 1));

    m_fd = open(
        outputPath.c_str(),
        O_CREAT | O_TRUNC | O_WRONLY,
        0664
    );

    if (m_fd < 0) {
        m_error = "Cannot create output file";
        return false;
    }

    m_codec = AMediaCodec_createEncoderByType("video/avc");

    if (!m_codec) {
        m_error = "H.264 encoder is unavailable";
        stop();
        return false;
    }

    AMediaFormat* format = AMediaFormat_new();

    if (!format) {
        m_error = "Cannot allocate MediaFormat";
        stop();
        return false;
    }

    AMediaFormat_setString(format, "mime", "video/avc");
    AMediaFormat_setInt32(format, "width", width);
    AMediaFormat_setInt32(format, "height", height);
    AMediaFormat_setInt32(format, "bitrate", bitrate);
    AMediaFormat_setInt32(format, "frame-rate", fps);
    AMediaFormat_setInt32(format, "i-frame-interval", 2);
    AMediaFormat_setInt32(format, "color-format", 0x7F000789);

    media_status_t status = AMediaCodec_configure(
        m_codec,
        format,
        nullptr,
        nullptr,
        AMEDIACODEC_CONFIGURE_FLAG_ENCODE
    );

    AMediaFormat_delete(format);

    if (status != AMEDIA_OK) {
        m_error = "H.264 encoder configuration failed";
        stop();
        return false;
    }

    status = AMediaCodec_createInputSurface(m_codec, &m_surface);

    if (status != AMEDIA_OK || !m_surface) {
        m_error = "Cannot create encoder input surface";
        stop();
        return false;
    }

    m_muxer = AMediaMuxer_new(
        m_fd,
        AMEDIAMUXER_OUTPUT_FORMAT_MPEG_4
    );

    if (!m_muxer) {
        m_error = "Cannot create MP4 muxer";
        stop();
        return false;
    }

    status = AMediaCodec_start(m_codec);

    if (status != AMEDIA_OK) {
        m_error = "Cannot start H.264 encoder";
        stop();
        return false;
    }

    m_running = true;

    m_thread = std::thread(
        &VideoEncoder::drainLoop,
        this
    );

    return true;
}

void VideoEncoder::drainLoop() {
    AMediaCodecBufferInfo info{};

    bool eos = false;

    while (!eos) {
        ssize_t index = AMediaCodec_dequeueOutputBuffer(
            m_codec,
            &info,
            10000
        );

        if (index == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            continue;
        }

        if (index == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            AMediaFormat* format =
                AMediaCodec_getOutputFormat(m_codec);

            if (!format) {
                m_error = "Cannot get encoder output format";
                m_failed = true;
                break;
            }

            m_track = static_cast<int>(
                AMediaMuxer_addTrack(m_muxer, format)
            );

            AMediaFormat_delete(format);

            if (m_track < 0 ||
                AMediaMuxer_start(m_muxer) != AMEDIA_OK) {
                m_error = "Cannot initialize MP4 video track";
                m_failed = true;
                break;
            }

            m_muxerStarted = true;
            continue;
        }

        if (index < 0)
            continue;

        size_t bufferSize = 0;

        uint8_t* buffer = AMediaCodec_getOutputBuffer(
            m_codec,
            static_cast<size_t>(index),
            &bufferSize
        );

        if (buffer && info.size > 0 && m_muxerStarted) {
            if (info.offset >= 0 &&
                static_cast<size_t>(info.offset + info.size) <= bufferSize) {

                AMediaCodecBufferInfo writeInfo = info;
                writeInfo.offset = 0;

                media_status_t result =
                    AMediaMuxer_writeSampleData(
                        m_muxer,
                        static_cast<size_t>(m_track),
                        buffer + info.offset,
                        &writeInfo
                    );

                if (result != AMEDIA_OK) {
                    m_error = "MP4 sample write failed";
                    m_failed = true;
                }
            }
        }

        if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM)
            eos = true;

        AMediaCodec_releaseOutputBuffer(
            m_codec,
            static_cast<size_t>(index),
            false
        );

        if (m_failed)
            break;
    }

    m_running = false;
}

void VideoEncoder::stop() {
    if (m_codec && m_running) {
        m_stopping = true;
        AMediaCodec_signalEndOfInputStream(m_codec);
    }

    if (m_thread.joinable())
        m_thread.join();

    if (m_codec) {
        AMediaCodec_stop(m_codec);
        AMediaCodec_delete(m_codec);
        m_codec = nullptr;
    }

    if (m_muxer) {
        if (m_muxerStarted)
            AMediaMuxer_stop(m_muxer);

        AMediaMuxer_delete(m_muxer);
        m_muxer = nullptr;
    }

    if (m_surface) {
        ANativeWindow_release(m_surface);
        m_surface = nullptr;
    }

    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }

    m_running = false;
    m_muxerStarted = false;
}

ANativeWindow* VideoEncoder::inputSurface() const {
    return m_surface;
}

bool VideoEncoder::isRunning() const {
    return m_running;
}

std::string VideoEncoder::error() const {
    return m_error;
}

}
