package com.gibbs.gplayer.render;

import android.opengl.Matrix;

import java.util.Arrays;

final public class GLMatrixHelper {
    private static final float MIN_SCALE = 1.0f;
    private static final float MAX_SCALE = 2.0f;

    private final float[] mvpMatrix = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
    };

    private float getTranslateX() {
        return mvpMatrix[12];
    }

    private void setTranslateX(float value) {
        mvpMatrix[12] = value;
    }

    private float getTranslateY() {
        return mvpMatrix[13];
    }

    private void setTranslateY(float value) {
        mvpMatrix[13] = value;
    }

    public float getTranslateZ() {
        return mvpMatrix[14];
    }

    private void setTranslateZ(float value) {
        mvpMatrix[14] = value;
    }

    private float getScaleX() {
        return mvpMatrix[0];
    }

    private void setScaleX(float value) {
        mvpMatrix[0] = value;
    }

    private float getScaleY() {
        return mvpMatrix[5];
    }

    private void setScaleY(float value) {
        mvpMatrix[5] = value;
    }

    private float getScaleZ() {
        return mvpMatrix[10];
    }

    private void setScaleZ(float value) {
        mvpMatrix[10] = value;
    }

    public void translate(float x, float y) {
        Matrix.translateM(mvpMatrix, 0, x, y, 0);
        adapterTranslate();
    }

    public void scale(float multiple) {
        Matrix.scaleM(mvpMatrix, 0, multiple, multiple, 0);
        adapterScale();
        adapterTranslate();
    }

    private void adapterTranslate() {
        if (getTranslateX() < -(getScaleX() - 1.0f)) {
            setTranslateX(-(getScaleX() - 1.0f));
        } else if (getTranslateX() > (getScaleX() - 1.0f)) {
            setTranslateX((getScaleX() - 1.0f));
        }
        if (getTranslateY() < -(getScaleY() - 1.0f)) {
            setTranslateY(-(getScaleY() - 1.0f));
        } else if (getTranslateY() > (getScaleY() - 1.0f)) {
            setTranslateY((getScaleY() - 1.0f));
        }
    }

    private void adapterScale() {
        if (getScaleX() < MIN_SCALE) {
            setScaleX(MIN_SCALE);
        } else if (getScaleX() > MAX_SCALE) {
            setScaleX(MAX_SCALE);
        }
        if (getScaleY() < MIN_SCALE) {
            setScaleY(MIN_SCALE);
        } else if (getScaleY() > MAX_SCALE) {
            setScaleY(MAX_SCALE);
        }
    }

    public float[] getMvpMatrix() {
        return mvpMatrix;
    }

    @Override
    public String toString() {
        return "GLMatrixHelper{" +
                "mvpMatrix=" + Arrays.toString(mvpMatrix) +
                '}';
    }
}
