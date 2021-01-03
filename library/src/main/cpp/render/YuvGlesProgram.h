/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_YUVGLESPROGRAM_H
#define GPLAYER_YUVGLESPROGRAM_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <base/Log.h>


#define VERTEX_SHADER \
        "attribute vec4 a_position;\n" \
        "attribute vec2 a_texCoord;\n" \
        "varying vec2 v_color;\n" \
        "uniform mat4 u_mvp;\n" \
        "void main() {\n" \
            "gl_Position = u_mvp * a_position;\n" \
            "v_color = a_texCoord;\n" \
        "}\n"

#define FRAGMENT_SHADER \
        "precision mediump float;\n" \
        "uniform sampler2D tex_y;\n" \
        "uniform sampler2D tex_u;\n" \
        "uniform sampler2D tex_v;\n" \
        "varying vec2 v_color;\n" \
        "void main() {\n" \
            "vec4 c = vec4((texture2D(tex_y, v_color).r - 16./255.) * 1.164);\n" \
            "vec4 U = vec4(texture2D(tex_u, v_color).r - 128./255.);\n" \
            "vec4 V = vec4(texture2D(tex_v, v_color).r - 128./255.);\n" \
            "c += V * vec4(1.596, -0.813, 0, 0);\n" \
            "c += U * vec4(0, -0.392, 2.017, 0);\n" \
            "c.a = 1.0;\n" \
            "gl_FragColor = c;\n" \
        "}\n"

class YuvGlesProgram {

private:
    GLfloat *SQUARE_VERTICES;
    GLfloat *COORD_VERTICES;
    GLfloat *MVP_MATRIX;

public:
    YuvGlesProgram();

    ~YuvGlesProgram();

    bool buildProgram();

    void buildTextures(uint8_t *y, uint8_t *u, uint8_t *v, uint32_t width, uint32_t height);

    void drawFrame();

private:
    GLuint createProgram(char *vertexSource, char *fragmentSource);

    GLuint loadShader(GLenum shaderType, char *source);

private:
    // program id
    GLuint _program;
    // texture id
    GLuint _textureI;
    GLuint _textureII;
    GLuint _textureIII;
    // texture index in gles
    GLuint _tIindex;
    GLuint _tIIindex;
    GLuint _tIIIindex;
    // handles
    GLuint _mvpHandle;
    GLuint _positionHandle, _coordHandle;
    GLuint _yhandle, _uhandle, _vhandle;
    GLuint _ytid, _utid, _vtid;
    // video width and height
    uint32_t _video_width;
    uint32_t _video_height;
};


#endif //GPLAYER_YUVGLESPROGRAM_H
