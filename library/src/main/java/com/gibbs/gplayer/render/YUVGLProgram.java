package com.gibbs.gplayer.render;

import android.opengl.GLES20;

import com.gibbs.gplayer.utils.LogUtils;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * step to use:<br/>
 * 1. new YUVGLProgram()<br/>
 * 2. buildProgram()<br/>
 * 3. buildTextures()<br/>
 * 4. drawFrame()<br/>
 */
class YUVGLProgram {

    private final String TAG = "YUVGLProgram";

    private final float[] squareVertices = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f}; // fullscreen
    private final float[] coordVertices = {0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};// whole-texture
    private final float[] mvpMatrix = {1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
    };

    // program id
    private int _program;
    // texture id
    private int _textureI;
    private int _textureII;
    private int _textureIII;
    // texture index in gles
    private int _tIindex;
    private int _tIIindex;
    private int _tIIIindex;
    // vertices on screen
    private float[] _vertices;
    // handles
    private int _mvpHandle = -1;
    private int _positionHandle = -1, _coordHandle = -1;
    private int _yhandle = -1, _uhandle = -1, _vhandle = -1;
    private int _ytid = -1, _utid = -1, _vtid = -1;
    // vertices buffer
    private ByteBuffer _vertice_buffer;
    private ByteBuffer _coord_buffer;
    // video width and height
    private int _video_width = -1;
    private int _video_height = -1;

    /**
     * position can only be 0~4:<br/>
     * fullscreen => 0<br/>
     * left-top => 1<br/>
     * right-top => 2<br/>
     * left-bottom => 3<br/>
     * right-bottom => 4
     */
    YUVGLProgram() {
        _vertices = squareVertices;
        _textureI = GLES20.GL_TEXTURE0;
        _textureII = GLES20.GL_TEXTURE1;
        _textureIII = GLES20.GL_TEXTURE2;
        _tIindex = 0;
        _tIIindex = 1;
        _tIIIindex = 2;

        initBuffers();
    }

    void buildProgram() {
        _program = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
        LogUtils.d(TAG, "_program = " + _program);

        _mvpHandle = GLES20.glGetUniformLocation(_program, "u_mvp");
        LogUtils.d(TAG, "_mvpHandle = " + _mvpHandle);
        checkGlError("glGetUniformLocation u_mvp");
        if (_mvpHandle == -1) {
            throw new RuntimeException("Could not get uniform location for u_mvp");
        }

        /*
         * get handle for "a_position" and "a_texCoord"
         */
        _positionHandle = GLES20.glGetAttribLocation(_program, "a_position");
        LogUtils.d(TAG, "_positionHandle = " + _positionHandle);
        checkGlError("glGetAttribLocation a_position");
        if (_positionHandle == -1) {
            throw new RuntimeException("Could not get attribute location for a_position");
        }

        _coordHandle = GLES20.glGetAttribLocation(_program, "a_texCoord");
        LogUtils.d(TAG, "_coordHandle = " + _coordHandle);
        checkGlError("glGetAttribLocation a_texCoord");
        if (_coordHandle == -1) {
            throw new RuntimeException("Could not get attribute location for a_texCoord");
        }

        /*
         * get uniform location for y/u/v, we pass data through these uniforms
         */
        _yhandle = GLES20.glGetUniformLocation(_program, "tex_y");
        LogUtils.d(TAG, "_yhandle = " + _yhandle);
        checkGlError("glGetUniformLocation tex_y");
        if (_yhandle == -1) {
            throw new RuntimeException("Could not get uniform location for tex_y");
        }
        _uhandle = GLES20.glGetUniformLocation(_program, "tex_u");
        LogUtils.d(TAG, "_uhandle = " + _uhandle);
        checkGlError("glGetUniformLocation tex_u");
        if (_uhandle == -1) {
            throw new RuntimeException("Could not get uniform location for tex_u");
        }
        _vhandle = GLES20.glGetUniformLocation(_program, "tex_v");
        LogUtils.d(TAG, "_vhandle = " + _vhandle);
        checkGlError("glGetUniformLocation tex_v");
        if (_vhandle == -1) {
            throw new RuntimeException("Could not get uniform location for tex_v");
        }
    }

    /**
     * build a set of textures, one for R, one for G, and one for B.
     */
    void buildTextures(Buffer y, Buffer u, Buffer v, int width, int height) {
        boolean videoSizeChanged = (width != _video_width || height != _video_height);
        if (videoSizeChanged) {
            _video_width = width;
            _video_height = height;
            LogUtils.d(TAG, "buildTextures videoSizeChanged: w=" + _video_width + " h=" + _video_height);
        }

        // building texture for Y data
        if (_ytid < 0 || videoSizeChanged) {
            if (_ytid >= 0) {
                LogUtils.d(TAG, "glDeleteTextures Y");
                GLES20.glDeleteTextures(1, new int[]{_ytid}, 0);
                checkGlError("glDeleteTextures");
            }
            // GLES20.glPixelStorei(GLES20.GL_UNPACK_ALIGNMENT, 1);
            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);
            checkGlError("glGenTextures");
            _ytid = textures[0];
            LogUtils.d(TAG, "glGenTextures Y = " + _ytid);
        }
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _ytid);
        checkGlError("glBindTexture");
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, _video_width, _video_height, 0,
                GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y);
        checkGlError("glTexImage2D");
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        // building texture for U data
        if (_utid < 0 || videoSizeChanged) {
            if (_utid >= 0) {
                LogUtils.d(TAG, "glDeleteTextures U");
                GLES20.glDeleteTextures(1, new int[]{_utid}, 0);
                checkGlError("glDeleteTextures");
            }
            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);
            checkGlError("glGenTextures");
            _utid = textures[0];
            LogUtils.d(TAG, "glGenTextures U = " + _utid);
        }
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _utid);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, _video_width / 2, _video_height / 2, 0,
                GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        // building texture for V data
        if (_vtid < 0 || videoSizeChanged) {
            if (_vtid >= 0) {
                LogUtils.d(TAG, "glDeleteTextures V");
                GLES20.glDeleteTextures(1, new int[]{_vtid}, 0);
                checkGlError("glDeleteTextures");
            }
            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);
            checkGlError("glGenTextures");
            _vtid = textures[0];
            LogUtils.d(TAG, "glGenTextures V = " + _vtid);
        }
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _vtid);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, _video_width / 2, _video_height / 2, 0,
                GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
    }

    /**
     * render the frame
     * the YUV data will be converted to RGB by shader.
     */
    void drawFrame() {
        GLES20.glUseProgram(_program);
        checkGlError("glUseProgram");

        GLES20.glVertexAttribPointer(_positionHandle, 2, GLES20.GL_FLOAT, false, 8, _vertice_buffer);
        checkGlError("glVertexAttribPointer mPositionHandle");
        GLES20.glEnableVertexAttribArray(_positionHandle);

        GLES20.glVertexAttribPointer(_coordHandle, 2, GLES20.GL_FLOAT, false, 8, _coord_buffer);
        checkGlError("glVertexAttribPointer maTextureHandle");
        GLES20.glEnableVertexAttribArray(_coordHandle);

        GLES20.glUniformMatrix4fv(_mvpHandle, 1, false, mvpMatrix, 0);

        // bind textures
        GLES20.glActiveTexture(_textureI);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _ytid);
        GLES20.glUniform1i(_yhandle, _tIindex);

        GLES20.glActiveTexture(_textureII);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _utid);
        GLES20.glUniform1i(_uhandle, _tIIindex);

        GLES20.glActiveTexture(_textureIII);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _vtid);
        GLES20.glUniform1i(_vhandle, _tIIIindex);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        GLES20.glFinish();

        GLES20.glDisableVertexAttribArray(_positionHandle);
        GLES20.glDisableVertexAttribArray(_coordHandle);
    }

    /**
     * create program and load shaders, fragment shader is very important.
     */
    private int createProgram(String vertexSource, String fragmentSource) {
        // create shaders
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        int pixelShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        // just check
        LogUtils.d(TAG, "vertexShader = " + vertexShader);
        LogUtils.d(TAG, "pixelShader = " + pixelShader);

        int program = GLES20.glCreateProgram();
        if (program != 0) {
            GLES20.glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            GLES20.glAttachShader(program, pixelShader);
            checkGlError("glAttachShader");
            GLES20.glLinkProgram(program);
            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
            if (linkStatus[0] != GLES20.GL_TRUE) {
                LogUtils.e(TAG, "Could not link program");
                LogUtils.e(TAG, GLES20.glGetProgramInfoLog(program));
                GLES20.glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

    /**
     * create shader with given source.
     */
    private int loadShader(int shaderType, String source) {
        int shader = GLES20.glCreateShader(shaderType);
        if (shader != 0) {
            GLES20.glShaderSource(shader, source);
            GLES20.glCompileShader(shader);
            int[] compiled = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
            if (compiled[0] == 0) {
                LogUtils.e(TAG, "Could not compile shader " + shaderType);
                LogUtils.e(TAG, "" + GLES20.glGetShaderInfoLog(shader));
                GLES20.glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    /**
     * these two buffers are used for holding vertices, screen vertices and texture vertices.
     */
    private void initBuffers() {
        _vertice_buffer = ByteBuffer.allocateDirect(squareVertices.length * 4);
        _vertice_buffer.order(ByteOrder.nativeOrder());
        _vertice_buffer.asFloatBuffer().put(squareVertices);
        _vertice_buffer.position(0);

        _coord_buffer = ByteBuffer.allocateDirect(coordVertices.length * 4);
        _coord_buffer.order(ByteOrder.nativeOrder());
        _coord_buffer.asFloatBuffer().put(coordVertices);
        _coord_buffer.position(0);
    }

    void updateMvp(float[] mvp) {
        for (int i = 0, size = mvpMatrix.length; i < size; i++) {
            mvpMatrix[i] = mvp[i];
        }
    }

    private void checkGlError(String op) {
        int error;
        if ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            LogUtils.e(TAG, " " + op + ": glError " + error);
            throw new RuntimeException(op + ": glError " + error);
        }
    }

    private static final String VERTEX_SHADER = "attribute vec4 a_position;\n" +
            "attribute vec2 a_texCoord;\n" +
            "varying vec2 v_color;\n" +
            "uniform mat4 u_mvp;\n" +
            "void main() {\n" +
            "gl_Position = u_mvp * a_position;\n" +
            "v_color = a_texCoord;\n" +
            "}\n";

    private static final String FRAGMENT_SHADER = "precision mediump float;\n" +
            "uniform sampler2D tex_y;\n" +
            "uniform sampler2D tex_u;\n" +
            "uniform sampler2D tex_v;\n" +
            "varying vec2 v_color;\n" +
            "void main() {\n" +
            "vec4 c = vec4((texture2D(tex_y, v_color).r - 16./255.) * 1.164);\n" +
            "vec4 U = vec4(texture2D(tex_u, v_color).r - 128./255.);\n" +
            "vec4 V = vec4(texture2D(tex_v, v_color).r - 128./255.);\n" +
            "c += V * vec4(1.596, -0.813, 0, 0);\n" +
            "c += U * vec4(0, -0.392, 2.017, 0);\n" +
            "c.a = 1.0;\n" +
            "gl_FragColor = c;\n" +
            "}\n";

}