#ifndef VIDEO_H
#define VIDEO_H

#include <GL/glut.h>
#include <string>

class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();
    
    bool loadVideo(const std::string& filename);
    void updateFrame();
    GLuint getTextureId() const;
    bool isLoaded() const;
    
private:
    GLuint textureId_;
    void* videoCapture_;
    unsigned char* frameBuffer_;
    int frameWidth_;
    int frameHeight_;
    bool isLoaded_;
};

#endif
