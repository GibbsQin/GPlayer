package com.gibbs.gplayer.sample;

import android.util.Log;


public class LogUtils {
    private static final boolean LOG_SWITCH = false;

    public static void e(final String tag, final String msg) {
        e(tag, msg, (Object[]) null);
    }

    public static void w(final String tag, final String msg) {
        w(tag, msg, (Object[]) null);
    }

    public static void i(final String tag, final String msg) {
        i(tag, msg, (Object[]) null);
    }

    public static void d(final String tag, final String msg) {
        d(tag, msg, (Object[]) null);
    }

    public static void v(final String tag, final String msg) {
        v(tag, msg, (Object[]) null);
    }

    public static void e(String tag, final String format, final Object... obj) {
        Log.e(tag, format);
    }

    public static void w(String tag, final String format, final Object... obj) {
        Log.w(tag, format);
    }

    public static void i(String tag, final String format, final Object... obj) {
        Log.i(tag, format);
    }

    public static void d(String tag, final String format, final Object... obj) {
        if (LOG_SWITCH) Log.d(tag, format);
    }

    public static void v(String tag, final String format, final Object... obj) {
        Log.v(tag, format);
    }
}
