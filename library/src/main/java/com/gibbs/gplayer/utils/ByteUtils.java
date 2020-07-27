package com.gibbs.gplayer.utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class ByteUtils {
    /**
     * int转4位byte  本方法适用于(低位在前，高位在后)的顺序
     *
     * @param value
     * @return
     */
    public static byte[] intToBytes(int value) {
        byte[] src = new byte[4];
        src[0] = (byte) (value & 0xFF);
        src[1] = (byte) ((value >> 8) & 0xFF);
        src[2] = (byte) ((value >> 16) & 0xFF);
        src[3] = (byte) ((value >> 24) & 0xFF);
        return src;
    }

    /**
     * 4位byte转int  本方法适用于(低位在前，高位在后)的顺序
     *
     * @param src
     * @param offset
     * @return
     */
    public static int bytesToInt(byte[] src, int offset) {
        int value;
        value = (int) ((src[offset] & 0xFF)
                | ((src[offset + 1] & 0xFF) << 8)
                | ((src[offset + 2] & 0xFF) << 16)
                | ((src[offset + 3] & 0xFF) << 24));
        return value;
    }


    /**
     * short到字节数组的转换！
     *
     * @param number
     * @return byte[]
     */
    public static byte[] shortToByte2(short number) {
        byte[] src = new byte[2];
        src[0] = (byte) (number & 0xFF);
        src[1] = (byte) ((number >> 8) & 0xFF);
        return src;
    }

    /**
     * 字节数组到short的转换！
     *
     * @param b      字节
     * @param offset 偏移量
     * @return short
     */
    public static short byte2ToShort(byte[] b, int offset) {
        short s = 0;
        short s0 = (short) (b[offset] & 0xFF);// 最低位
        short s1 = (short) (b[offset + 1] & 0xFF);
        s1 <<= 8;
        s = (short) (s0 | s1);
        return s;
    }

    /**
     * long转8位byte  本方法适用于(低位在前，高位在后)的顺序
     *
     * @param value
     * @return
     */
    public static byte[] longToBytes(long value) {
        byte[] src = new byte[8];
        src[0] = (byte) (value & 0xFF);
        src[1] = (byte) ((value >> 8) & 0xFF);
        src[2] = (byte) ((value >> 16) & 0xFF);
        src[3] = (byte) ((value >> 24) & 0xFF);
        src[4] = (byte) ((value >> 32) & 0xFF);
        src[5] = (byte) ((value >> 40) & 0xFF);
        src[6] = (byte) ((value >> 48) & 0xFF);
        src[7] = (byte) ((value >> 56) & 0xFF);
        return src;
    }

    /**
     * 8位byte转long  本方法适用于(低位在前，高位在后)的顺序
     *
     * @return
     */
    public static long bytesTolong(byte[] byteArray, int offset) {
        ByteBuffer buffer = ByteBuffer.allocate(8);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        buffer.put(byteArray, offset, 8);
        buffer.flip();
        return buffer.getLong();
    }

    /**
     * 获取低32位
     *
     * @param value
     * @return
     */
    public static long lowBit32(long value) {
        return (value & 0xFFFFFFFFL);
    }
}
