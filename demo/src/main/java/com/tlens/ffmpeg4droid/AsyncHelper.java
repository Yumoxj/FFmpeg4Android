package com.tlens.ffmpeg4droid;

import androidx.annotation.NonNull;

import java.util.concurrent.Callable;
import java.util.concurrent.Future;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

public class AsyncHelper {
    // final Thread thread = new Thread(runnable);
    // thread.setPriority(Process.THREAD_PRIORITY_DEFAULT
    //        + Process.THREAD_PRIORITY_LESS_FAVORABLE);
    private static final ThreadFactory THREAD_FACTORY = Thread::new;

    private static final ThreadPoolExecutor sThreadPool
            = new ThreadPoolExecutor(1, 1,
            30, TimeUnit.SECONDS,
            new LinkedBlockingQueue<>(), THREAD_FACTORY);
    public static Future<?> doInBackground(@NonNull Runnable task) {
        return sThreadPool.submit(task);
    }
    public static <T> Future<T> doInBackground(Callable<T> task) {
        return sThreadPool.submit(task);
    }
}
