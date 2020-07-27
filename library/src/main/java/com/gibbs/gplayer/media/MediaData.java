package com.gibbs.gplayer.media;

import java.nio.ByteBuffer;

public class MediaData implements Comparable<MediaData> {
    public static final int FLAG_KEY_FRAME = 0x00000001;
    public static final int FLAG_KEY_EXTRA_DATA = 0x00000002;
    public static final int FLAG_KEY_RENDERED = 0x00000004;

    public ByteBuffer data;

    public int size;

    public ByteBuffer data1;

    public int size1;

    public ByteBuffer data2;

    public int size2;

    public long pts;

    public long dts;

    public int width;

    public int height;

    /**
     * 帧类型的组合，与FLAG_KEY_相关
     */
    public int flag;

    public MediaData() {

    }

    public MediaData(ByteBuffer data, int size, long pts) {
        this.data = data;
        this.size = size;
        this.pts = pts;
    }

    public MediaData(MediaData srcData) {
        if (srcData.size > 0) {
            data = ByteBuffer.allocate(srcData.size);
            data.put(srcData.data);
            data.position(0);
            size = srcData.size;
        }
        if (srcData.size1 > 0) {
            data1 = ByteBuffer.allocate(srcData.size1);
            data1.put(srcData.data1);
            data1.position(0);
            size1 = srcData.size1;
        }
        if (srcData.size2 > 0) {
            data2 = ByteBuffer.allocate(srcData.size2);
            data2.put(srcData.data2);
            data2.position(0);
            size2 = srcData.size2;
        }
        pts = srcData.pts;
        dts = srcData.dts;
        width = srcData.width;
        height = srcData.height;
        flag = srcData.flag;
    }

    @Override
    public int compareTo(MediaData o) {
        if (pts > o.pts) {
            return 1;
        } else if (pts < o.pts) {
            return -1;
        }
        return 0;
    }
}
