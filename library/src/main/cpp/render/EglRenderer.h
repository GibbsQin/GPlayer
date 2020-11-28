#ifndef EGL_RENDERER_H
#define EGL_RENDERER_H

#include <cstdint>
#include <unistd.h>
#include <pthread.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>
#include <base/Log.h>

class EglRenderer {

public:
    EglRenderer();

    ~EglRenderer();

    void setWindow(ANativeWindow *window);

    bool initialize();

    void destroy();

    void drawFrame();

private:
    ANativeWindow *_window{};

    EGLDisplay _display{};
    EGLSurface _surface{};
    EGLContext _context{};
};

#endif // EGL_RENDERER_H
