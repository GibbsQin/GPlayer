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

    bool initialize();

    void destroy();

    void buildTextures(char *y, char *u, char *v, uint32_t width, uint32_t height);

    void drawFrame();

private:
    YuvGlesProgram *_glProgram;
    ANativeWindow *_window{};

    EGLDisplay _display{};
    EGLSurface _surface{};
    EGLContext _context{};
};

#endif // EGL_RENDERER_H
