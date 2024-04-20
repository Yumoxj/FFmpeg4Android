package com.tlens.ff4droidlibrary;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

public class PermissionsActivity extends Activity {
    private static final String[] SDK_32_PERMISSIONS = {
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    };

    @RequiresApi(api = Build.VERSION_CODES.TIRAMISU)
    private static final String[] SDK_32_PLUS_PERMISSIONS = {
            Manifest.permission.READ_MEDIA_AUDIO,
            Manifest.permission.READ_MEDIA_VIDEO,
    };

    @RequiresApi(api = Build.VERSION_CODES.UPSIDE_DOWN_CAKE)
    private static final String[] SDK_34_PLUS_PERMISSIONS = {
            Manifest.permission.READ_MEDIA_AUDIO,
            Manifest.permission.READ_MEDIA_VIDEO,
            Manifest.permission.READ_MEDIA_VISUAL_USER_SELECTED
    };

    private static final int currentSdkVersion = Build.VERSION.SDK_INT;
    private static final String[] PERMISSIONS = currentSdkVersion <= Build.VERSION_CODES.S_V2 ?
            SDK_32_PERMISSIONS : (currentSdkVersion >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE ?
            SDK_34_PLUS_PERMISSIONS : SDK_32_PLUS_PERMISSIONS);

    private static final int REQUEST_CODE = 42;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (currentSdkVersion >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                startActivity(intent);
            }
        }

        // Check again, just in case.
        if (!hasRequiredPermissions(this)) {
            requestPermissions(PERMISSIONS, REQUEST_CODE);
        } else {
            finish();
        }
    }

    @Override
    public void onRequestPermissionsResult(int request,
                                           @NonNull String[] permissions,
                                           @NonNull int[] results) {
        finish();
    }

    public static boolean hasRequiredPermissions(Context context) {
        for (String permission : PERMISSIONS) {
            if (context.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }
}
