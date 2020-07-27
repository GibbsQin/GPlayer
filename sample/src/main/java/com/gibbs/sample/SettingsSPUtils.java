package com.gibbs.sample;

import android.content.Context;
import android.content.SharedPreferences;

public class SettingsSPUtils {
    private static final String SP_FILE = "com.gibbs.gplayer_preferences";

    public static final String CODEC_SOURCE = "codec_source";
    public static final String CODEC_TYPE = "codec_type";
    public static final String GPLAYER_STYLE = "gplayer_style";

    private static class SPHolder {
        private static final SettingsSPUtils INSTANCE = new SettingsSPUtils();
    }

    public static SettingsSPUtils getInstance() {
        return SPHolder.INSTANCE;
    }

    public String getString(Context context, String key) {
        return getString(context, key, "");
    }

    public String getString(Context context, String key, String defaultValue) {
        SharedPreferences sf = context.getSharedPreferences(SP_FILE, Context.MODE_PRIVATE);
        return sf.getString(key, defaultValue);
    }

    public String getCodecLayer(Context context) {
        return getString(context, CODEC_SOURCE);
    }

    public String getCodecType(Context context) {
        return getString(context, CODEC_TYPE);
    }

    public String getGPlayerStyle(Context context) {
        return getString(context, GPLAYER_STYLE, "external");
    }

    public boolean isDecodeSource(Context context) {
        return getCodecLayer(context).equals("1");
    }

    public boolean isMediaCodec(Context context) {
        return getCodecType(context).equals("2");
    }
}
