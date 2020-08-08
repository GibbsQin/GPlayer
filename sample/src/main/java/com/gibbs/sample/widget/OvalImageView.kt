package com.gibbs.sample.widget

import android.content.Context
import android.content.res.Resources
import android.graphics.Canvas
import android.graphics.Path
import android.graphics.RectF
import android.util.AttributeSet
import android.util.TypedValue
import androidx.appcompat.widget.AppCompatImageView

class OvalImageView : AppCompatImageView {
    private val circleValue = 8f
    private val rids = floatArrayOf(dp2px(circleValue).toFloat(), dp2px(circleValue).toFloat(), dp2px(circleValue).toFloat(), dp2px(circleValue).toFloat(),
            dp2px(circleValue).toFloat(), dp2px(circleValue).toFloat(), dp2px(circleValue).toFloat(), dp2px(circleValue).toFloat())
    private val path = Path()
    private val rect = RectF()

    constructor(context: Context?) : super(context)
    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs)
    constructor(context: Context?, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr)

    override fun onDraw(canvas: Canvas) {
        val w = this.width
        val h = this.height
        rect[0f, 0f, w.toFloat()] = h.toFloat()
        path.addRoundRect(rect, rids, Path.Direction.CW)
        canvas.clipPath(path)
        super.onDraw(canvas)
    }

    private fun dp2px(dp: Float): Int {
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp,
                Resources.getSystem().displayMetrics).toInt()
    }
}