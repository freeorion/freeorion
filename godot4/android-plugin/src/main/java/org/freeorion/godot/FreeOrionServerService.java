package org.freeorion.godot;

import android.content.Context;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Process;
import android.util.Log;

/**
 * Hosts the FreeOrion server in a dedicated Android process.
 */
public final class FreeOrionServerService extends Service {
    private static final String TAG = "FreeOrionServerService";

    private static volatile boolean NATIVE_STARTED = false;
    private static volatile boolean DESTROYED = false;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "FreeOrion server service created (pid=" + Process.myPid() + ")");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "FreeOrion server service started; keeping it running");
        String[] serverArgs = null;
        if (intent != null) {
            serverArgs = intent.getStringArrayExtra("args");
        }

        final String[] finalArgs = serverArgs;
        new Thread(() -> {
            if (DESTROYED)
                return;
            System.loadLibrary("freeoriond");
            if (DESTROYED)
                return;
            NATIVE_STARTED = true;
            startNativeService(this, finalArgs);
        }, "FreeOrionServerThread").start();
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
            new Thread(() -> stopNativeService(), "FreeOrionServerThreadStop").start();
        }
        super.onDestroy();
        Log.i(TAG, "FreeOrion server service destroyed");
    }

    private static native int startNativeService(Context activity, String[] args);
    private static native void stopNativeService();
}
