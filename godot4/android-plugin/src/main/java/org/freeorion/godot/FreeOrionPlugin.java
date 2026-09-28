package org.freeorion.godot;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;
import android.view.RoundedCorner;
import android.view.View;
import android.view.WindowInsets;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;
import org.godotengine.godot.plugin.UsedByGodot;

import java.util.ArrayList;
import java.util.List;

/**
 * FreeOrion Android plugin.
 */
public final class FreeOrionPlugin extends GodotPlugin {

    private static final String TAG = "FreeOrionPlugin";

    public FreeOrionPlugin(Godot godot) {
        super(godot);
        Activity activity = getActivity();
        if (activity != null) {
            System.loadLibrary("freeoriongodot");
            setAndroidContext(activity);
        }
    }

    @Override
    public String getPluginName() {
        return "FreeOrion";
    }

    @Override
    public List<String> getCommandLineParams(List<String> params) {
        List<String> result = new ArrayList<>(params);
        Activity activity = getActivity();
        if (activity != null) {
            if (activity.getIntent().getBooleanExtra("quickstart", false)) {
                result.add("--quickstart");
            }
            int autoAdvanceNTurns = activity.getIntent().getIntExtra("auto-advance-n-turns", 0);
            if (autoAdvanceNTurns > 0) {
                result.add("--auto-advance-n-turns");
                result.add(Integer.toString(autoAdvanceNTurns));
            }
            int setupAIPlayerCount = activity.getIntent().getIntExtra("setup.ai.player.count", -1);
            if (setupAIPlayerCount >= 0) {
                result.add("--setup.ai.player.count");
                result.add(Integer.toString(setupAIPlayerCount));
            }
        }
        return result;
    }

    private static native void setAndroidContext(Context context);

    @UsedByGodot
    public void startServer(String[] args) {
        Activity activity = getActivity();
        if (activity == null) {
            Log.w(TAG, "No activity available; cannot start server service");
            return;
        }
        Log.i(TAG, "Starting FreeOrionServerService");
        Intent intent = new Intent(activity, FreeOrionServerService.class);
        intent.putExtra("args", args);
        activity.startService(intent);
    }

    @UsedByGodot
    public void stopServer() {
        Activity activity = getActivity();
        if (activity == null) {
            Log.w(TAG, "No activity available; cannot stop server service");
            return;
        }
        Log.i(TAG, "Stopping FreeOrionServerService");
        activity.stopService(new Intent(activity, FreeOrionServerService.class));
    }

    @UsedByGodot
    public int[] getRoundedCornerRadii() {
        Activity activity = getActivity();
        if (activity == null) {
            Log.w(TAG, "No activity available; cannot query rounded corner");
            return new int[] {0, 0, 0, 0};
        }
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            Log.w(TAG, "Old Android version; cannot query rounded corner");
            return new int[] {0, 0, 0, 0};
        }

        View decorView = activity.getWindow().getDecorView();
        WindowInsets insets = decorView.getRootWindowInsets();
        if (insets == null) {
            Log.w(TAG, "Insets missing; cannot query rounded corner");
            return new int[] {0, 0, 0, 0};
        }

        int topLeft = getRadius(insets.getRoundedCorner(RoundedCorner.POSITION_TOP_LEFT));
        int topRight = getRadius(insets.getRoundedCorner(RoundedCorner.POSITION_TOP_RIGHT));
        int bottomRight = getRadius(insets.getRoundedCorner(RoundedCorner.POSITION_BOTTOM_RIGHT));
        int bottomLeft = getRadius(insets.getRoundedCorner(RoundedCorner.POSITION_BOTTOM_LEFT));

        return new int[] {topLeft, topRight, bottomRight, bottomLeft};
    }

    private int getRadius(RoundedCorner corner)
    { return corner != null ? corner.getRadius() : 0; }
}
