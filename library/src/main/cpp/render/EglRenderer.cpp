#include "EglRenderer.h"

#define TAG "EglRenderer"

EglRenderer::EglRenderer() {
    _glProgram = new YuvGlesProgram();
}

EglRenderer::~EglRenderer() {
    delete _glProgram;
}

void EglRenderer::setWindow(ANativeWindow *window) {
    _window = window;
}

bool EglRenderer::initialize() {
    const EGLint attrib[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;
    EGLSurface surface;
    EGLContext context;
    EGLint width;
    EGLint height;

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOGE(TAG, "eglGetDisplay() returned error %d", eglGetError());
        return false;
    }

    if (!eglInitialize(display, nullptr, nullptr)) {
        LOGE(TAG, "eglInitialize() returned error %d", eglGetError());
        return false;
    }

    if (!eglChooseConfig(display, attrib, &config, 1, &numConfigs)) {
        LOGE(TAG, "eglChooseConfig() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        LOGE(TAG, "eglGetConfigAttrib() returned error %d", eglGetError());
        destroy();
        return false;
    }

    ANativeWindow_setBuffersGeometry(_window, 0, 0, format);

    if (!(surface = eglCreateWindowSurface(display, config, _window, nullptr))) {
        LOGE(TAG, "eglCreateWindowSurface() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!(context = eglCreateContext(display, config, nullptr, nullptr))) {
        LOGE(TAG, "eglCreateContext() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        LOGE(TAG, "eglMakeCurrent() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
        LOGE(TAG, "eglQuerySurface() returned error %d", eglGetError());
        destroy();
        return false;
    }

    _display = display;
    _surface = surface;
    _context = context;

    _glProgram->buildProgram();

    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, width, height);

    return true;
}

void EglRenderer::destroy() {
    LOGI(TAG, "Destroying context");

    eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(_display, _context);
    eglDestroySurface(_display, _surface);
    eglTerminate(_display);

    _display = EGL_NO_DISPLAY;
    _surface = EGL_NO_SURFACE;
    _context = EGL_NO_CONTEXT;
}

void EglRenderer::buildTextures(char *y, char *u, char *v, uint32_t width, uint32_t height) {
    _glProgram->buildTextures(y, u, v, width, height);
}

void EglRenderer::drawFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _glProgram->drawFrame();

    if (_display) {
        if (!eglSwapBuffers(_display, _surface)) {
            LOGE(TAG, "eglSwapBuffers() returned error %d", eglGetError());
        }
    }
}
