package com.gibbs.gplayer.utils;

import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.text.TextUtils;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;

public class MediaExtractorUtils {
    public static void printMediaInfo(String url) {
        LogUtils.i("MediaExtractorUtils", "url = " + url);
        MediaExtractor extractor = new MediaExtractor();
        try {
            extractor.setDataSource(url);
        } catch (IOException e) {
            e.printStackTrace();
        }
        int numTracks = extractor.getTrackCount();
        for (int i = 0; i < numTracks; ++i) {
            MediaFormat format = extractor.getTrackFormat(i);
            LogUtils.i("MediaExtractorUtils", "MediaFormat = " + format.toString());
            ByteBuffer csd0 = format.getByteBuffer("csd-0");
            if (csd0 != null) {
                LogUtils.i("MediaExtractorUtils", "csd0 = " + Arrays.toString(csd0.array()));
            }
            ByteBuffer csd1 = format.getByteBuffer("csd-1");
            if (csd1 != null) {
                LogUtils.i("MediaExtractorUtils", "csd1 = " + Arrays.toString(csd1.array()));
            }
            ByteBuffer csd2 = format.getByteBuffer("csd-2");
            if (csd2 != null) {
                LogUtils.i("MediaExtractorUtils", "csd2 = " + Arrays.toString(csd2.array()));
            }
        }
        extractor.release();
    }

    public static MediaFormat getVideoFormat(String url) {
        LogUtils.i("MediaExtractorUtils", "url = " + url);
        MediaExtractor extractor = new MediaExtractor();
        try {
            extractor.setDataSource(url);
        } catch (IOException e) {
            e.printStackTrace();
        }
        int numTracks = extractor.getTrackCount();
        for (int i = 0; i < numTracks; ++i) {
            MediaFormat format = extractor.getTrackFormat(i);
            String mime = format.getString(MediaFormat.KEY_MIME);
            if (mime != null && mime.startsWith("video/")) {
                return format;
            }
        }
        extractor.release();
        return null;
    }

    public static MediaFormat getFormatByMine(String url, String targetMime) {
        LogUtils.i("MediaExtractorUtils", "url = " + url + ", targetMime = " + targetMime);
        MediaExtractor extractor = new MediaExtractor();
        try {
            extractor.setDataSource(url);
        } catch (IOException e) {
            e.printStackTrace();
        }
        int numTracks = extractor.getTrackCount();
        for (int i = 0; i < numTracks; ++i) {
            MediaFormat format = extractor.getTrackFormat(i);
            String mime = format.getString(MediaFormat.KEY_MIME);
            if (TextUtils.equals(targetMime, mime)) {
                return format;
            }
        }
        extractor.release();
        return null;
    }
}
