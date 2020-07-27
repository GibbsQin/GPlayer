package com.gibbs.gplayer.render;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;


public abstract class GestureGLSurfaceView extends GLSurfaceView {
    private static final String TAG = "GestureGLSurfaceView";

    private static final boolean USING_CUSTOMS_SCALE_GESTURE_DETECTOR = true;

    private static final int MINX = 50;
    private static final int MINY = 25;
    public static final int USR_CMD_OPTION_PTZ_TURN_LEFT = 0;
    public static final int USR_CMD_OPTION_PTZ_TURN_RIGHT = 1;
    public static final int USR_CMD_OPTION_PTZ_TURN_UP = 2;
    public static final int USR_CMD_OPTION_PTZ_TURN_DOWN = 3;

    private ScaleGestureDetector mScaleGestureDetector;
    private GestureDetector mGestureDetector;
    private OnSingleTapUp mSingleTapUpListener;
    private OnLongPressListener mLongPressedListener;
    private OnFlingListener mFlingListener;
    private OnDownListener mDownListener;
    private float mZoomCenterX;
    private float mZoomCenterY;
    private int mCurrentPointerCount;
    private float mLastPointerDistance;

    private VideoRender mRenderer;
    private GLMatrixHelper mGLMatrixHelper;
    private int mViewWidth;
    private int mViewHeight;

    public GestureGLSurfaceView(Context context) {
        super(context);
        initView();
    }

    public GestureGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    private void initView() {
        initGestureDetector();
    }

    private void initGestureDetector() {
        if (!USING_CUSTOMS_SCALE_GESTURE_DETECTOR) {
            mScaleGestureDetector = new ScaleGestureDetector(getContext(), new ScaleGestureDetector.SimpleOnScaleGestureListener() {
                @Override
                public boolean onScale(ScaleGestureDetector scaleGestureDetector) {
                    onZoom(mZoomCenterX, mZoomCenterY, scaleGestureDetector.getScaleFactor());
                    return true;
                }
            });
        }

        mGestureDetector = new GestureDetector(getContext(), new GestureDetector.SimpleOnGestureListener() {
            @Override
            public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
                if (mCurrentPointerCount == 1) {
                    onMove(-distanceX, distanceY);
                    return true;
                } else {
                    return false;
                }
            }

            @Override
            public boolean onSingleTapUp(MotionEvent e) {
                if (mSingleTapUpListener != null) {
                    mSingleTapUpListener.onSingleTapUp(e);
                    return true;
                } else {
                    return false;
                }
            }

            @Override
            public boolean onDown(MotionEvent e) {
                if (mCurrentPointerCount == 1) {
                    if (mDownListener != null) {
                        mDownListener.onDown(e);
                    }
                }
                return true;
            }

            @Override
            public void onLongPress(MotionEvent e) {
                if (mCurrentPointerCount == 1) {
                    if (mLongPressedListener != null) {
                        mLongPressedListener.onLongPress(e);
                    }
                }
            }

            @Override
            public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
                int id = -1;
                float distance;
                boolean ishorizontal = false;
                if ((Math.abs(e2.getX() - e1.getX())) > (Math.abs(e2.getY()
                        - e1.getY()))) {
                    ishorizontal = true;
                }

                if (ishorizontal) {
                    distance = e2.getX() - e1.getX();
                    if (Math.abs(distance) > dip2px(getContext(), MINX)) {
                        if (distance > 0) {
                            id = USR_CMD_OPTION_PTZ_TURN_RIGHT;
                        } else {
                            id = USR_CMD_OPTION_PTZ_TURN_LEFT;
                        }
                    }
                } else {
                    distance = e2.getY() - e1.getY();
                    if (Math.abs(distance) > dip2px(getContext(), MINY)) {
                        if (distance > 0) {
                            id = USR_CMD_OPTION_PTZ_TURN_UP;
                        } else {
                            id = USR_CMD_OPTION_PTZ_TURN_DOWN;
                        }
                    }
                }

                if (id != -1) {
                    if (mFlingListener != null) {
                        mFlingListener.onFling(id);
                    }
                }
                return true;
            }
        });
    }

    private int dip2px(Context context, int dipValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (dipValue * scale + 0.5f);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_POINTER_1_DOWN ||
                event.getAction() == MotionEvent.ACTION_POINTER_2_DOWN ||
                event.getAction() == MotionEvent.ACTION_POINTER_3_DOWN ||
                event.getAction() == MotionEvent.ACTION_DOWN) {
            mCurrentPointerCount = event.getPointerCount();
            if (USING_CUSTOMS_SCALE_GESTURE_DETECTOR) {
                if (mCurrentPointerCount == 2) {
                    mLastPointerDistance = getDistance(event);
                }
            }
        }
        if (event.getPointerCount() == 1) {
            mGestureDetector.onTouchEvent(event);
            return true;
        } else if(event.getPointerCount() == 2) {
            mZoomCenterX = (event.getX(0) + event.getX(1)) / 2;
            mZoomCenterY = (event.getY(0) + event.getY(1)) / 2;
            if (!USING_CUSTOMS_SCALE_GESTURE_DETECTOR) {
                mScaleGestureDetector.onTouchEvent(event);
            } else {
                float currentDistance = getDistance(event);
                float gapLength = currentDistance - mLastPointerDistance;
                if (Math.abs(gapLength) > 5f) {
                    float scale = currentDistance / mLastPointerDistance;
                    mLastPointerDistance = currentDistance;
                    onZoom(mZoomCenterX, mZoomCenterY, scale);
                }
            }
            return true;
        }
        return super.onTouchEvent(event);
    }

    private float getDistance(MotionEvent event) {
        float x = 0f;
        float y = 0f;
        //避免出现：java.lang.IllegalArgumentException: pointerIndex out of range
        try {
            x = event.getX(0) - event.getX(1);
            y = event.getY(0) - event.getY(1);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
        return (float) Math.sqrt(x * x + y * y);
    }

    public void setSingleTapUpListener(OnSingleTapUp listener) {
        mSingleTapUpListener = listener;
    }

    public void setLongPressedListener(OnLongPressListener listener) {
        mLongPressedListener = listener;
    }

    public void setOnFlingListener(OnFlingListener listener) {
        mFlingListener = listener;
    }

    public void setOnDownListener(OnDownListener listener) {
        mDownListener = listener;
    }

    public interface OnSingleTapUp {
        void onSingleTapUp(MotionEvent e);
    }

    public interface OnLongPressListener {
        void onLongPress(MotionEvent e);
    }

    public interface OnFlingListener {
        void onFling(int direction);
    }

    public interface OnDownListener {
        void onDown(MotionEvent e);
    }

    protected void onZoom(float x, float y, float scale) {
        if (mRenderer != null) {
            mGLMatrixHelper.scale(scale);
            mRenderer.updateMvp(mGLMatrixHelper.getMvpMatrix());
        }
    }

    protected void onMove(float moveX, float moveY) {
        if (mRenderer != null) {
            mGLMatrixHelper.translate(moveX / mViewWidth, moveY / mViewHeight);
            mRenderer.updateMvp(mGLMatrixHelper.getMvpMatrix());
        }
    }

    @Override
    public void setRenderer(Renderer renderer) {
        super.setRenderer(renderer);
        if (renderer instanceof VideoRender) {
            mRenderer = (VideoRender) renderer;
            mGLMatrixHelper = new GLMatrixHelper();
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        mViewWidth = MeasureSpec.getSize(widthMeasureSpec);
        mViewHeight = MeasureSpec.getSize(heightMeasureSpec);
    }
}
