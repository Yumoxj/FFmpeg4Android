package com.tlens.ff4droidlibrary;

import androidx.annotation.NonNull;

public class FFTools {
    public interface OnMsgListener {
        void onMsg(String msg, int level);
    }

    private static OnMsgListener msgListener = (msg, level) -> {
    };

    static {
        System.loadLibrary("ff4droidlibrary");
    }

    public static void onMsgCallback(String msg, int level) {
        if (msg != null && !msg.isEmpty() && msgListener != null) {
            msgListener.onMsg(msg, level);
        }
    }

    static public void setMsgListener(@NonNull OnMsgListener msgListener) {
        FFTools.msgListener = msgListener;
    }

    static public native void init();
    static public native void deinit();
    static public native String version();
    static public synchronized native int ffmpeg(String[] commands);
}
