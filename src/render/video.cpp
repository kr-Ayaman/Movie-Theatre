#include "render/video.h"
#include <opencv2/opencv.hpp>
#include <iostream>

VideoPlayer::VideoPlayer() 
    : textureId_(0), videoCapture_(nullptr), frameBuffer_(nullptr),
      frameWidth_(1024), frameHeight_(576), isLoaded_(false) {
    glGenTextures(1, &textureId_);
    frameBuffer_ = new unsigned char[frameWidth_ * frameHeight_ * 3];
}

VideoPlayer::~VideoPlayer() {
    if (videoCapture_) {
        cv::VideoCapture* cap = static_cast<cv::VideoCapture*>(videoCapture_);
        cap->release();
        delete cap;
    }
    if (frameBuffer_) {
        delete[] frameBuffer_;
    }
    if (textureId_) {
        glDeleteTextures(1, &textureId_);
    }
}

bool VideoPlayer::loadVideo(const std::string& filename) {
    std::cout << "Loading video: " << filename << std::endl;
    
    cv::VideoCapture* cap = new cv::VideoCapture(filename);
    if (!cap->isOpened()) {
        std::cerr << "Failed to open video file: " << filename << std::endl;
        delete cap;
        return false;
    }
    
    videoCapture_ = cap;
    isLoaded_ = true;
    std::cout << "Video loaded successfully" << std::endl;
    
    // Setup initial texture
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth_, frameHeight_, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

void VideoPlayer::updateFrame() {
    if (!videoCapture_ || !isLoaded_) {
        return;
    }
    
    cv::VideoCapture* cap = static_cast<cv::VideoCapture*>(videoCapture_);
    cv::Mat frame;
    
    if (cap->read(frame)) {
        cv::Mat resized;
        cv::resize(frame, resized, cv::Size(frameWidth_, frameHeight_));
        cv::Mat rgb;
        cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);

        glBindTexture(GL_TEXTURE_2D, textureId_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth_, frameHeight_, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        // Loop video
        cap->set(cv::CAP_PROP_POS_FRAMES, 0);
    }
}

GLuint VideoPlayer::getTextureId() const {
    return textureId_;
}

bool VideoPlayer::isLoaded() const {
    return isLoaded_;
}
