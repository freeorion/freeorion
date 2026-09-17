package org.freeorion.godot;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Process;
import android.content.Context;
import android.util.Log;

public abstract class FreeOrionAIService extends Service {
    private static final String TAG = "FreeOrionAIService";

    final int number;

    private static volatile boolean NATIVE_STARTED = false;
    private static volatile boolean DESTROYED = false;

    protected FreeOrionAIService(int number) {
        this.number = number;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG + number, "FreeOrion AI " + number + " service created (pid=" + Process.myPid() + ")");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG + number, "FreeOrion AI " + number + " service started; keeping it running");
        String[] aiArgs = null;
        if (intent != null) {
            aiArgs = intent.getStringArrayExtra("args");
        }

        final String[] finalArgs = aiArgs;
        new Thread(() -> {
            if (DESTROYED)
                return;
            System.loadLibrary("freeorionca");
            if (DESTROYED)
                return;
            NATIVE_STARTED = true;
            startNativeService(this, finalArgs);
        }, "FreeOrionAIThread").start();
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        DESTROYED = true;
        if (NATIVE_STARTED) {
            new Thread(() -> stopNativeService(), "FreeOrionAIThreadStop").start();
        }
        super.onDestroy();
        Log.i(TAG + number, "FreeOrion AI " + number + " service destroyed");
    }

    private static native int startNativeService(Context activity, String[] args);
    private static native void stopNativeService();
}
