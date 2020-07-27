package com.gibbs.sample.widget;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Path;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.TypedValue;

import androidx.appcompat.widget.AppCompatImageView;

public class OvalImageView extends AppCompatImageView {
    private final float circleValue = 8;
    private float[] rids = {dp2px(circleValue), dp2px(circleValue), dp2px(circleValue), dp2px(circleValue),
            dp2px(circleValue), dp2px(circleValue), dp2px(circleValue), dp2px(circleValue)};

    private Path path = new Path();
    private RectF rect = new RectF();

    public OvalImageView(Context context) {
        super(context);
    }

    public OvalImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public OvalImageView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        int w = this.getWidth();
        int h = this.getHeight();
        rect.set(0, 0, w, h);
        path.addRoundRect(rect, rids, Path.Direction.CW);
        canvas.clipPath(path);
        super.onDraw(canvas);
    }

    private int dp2px(float dp) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp,
                Resources.getSystem().getDisplayMetrics());
    }
}
