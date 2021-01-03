#ifndef EGL_RENDERER_H
#define EGL_RENDERER_H

#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <base/Log.h>
#include "YuvGlesProgram.h"

class EglRenderer {

public:
    EglRenderer();

    ~EglRenderer();

    void setWindow(ANativeWindow *window);

    void setVideoSize(int width, int height);

    bool initialize();

    void destroy();

    void buildTextures(uint8_t *y, uint8_t *u, uint8_t *v, uint32_t width, uint32_t height);

    void drawFrame();

private:
    YuvGlesProgram *_glProgram;
    ANativeWindow *_window{};
    int videoWidth = 0;
    int videoHeight = 0;

    EGLDisplay _display{};
    EGLSurface _surface{};
    EGLContext _context{};
};

#endif // EGL_RENDERER_H
