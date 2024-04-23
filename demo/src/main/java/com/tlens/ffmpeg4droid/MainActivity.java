package com.tlens.ffmpeg4droid;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.tlens.ff4droidlibrary.FFTools;
import com.tlens.ff4droidlibrary.PermissionsActivity;

public class MainActivity extends AppCompatActivity {
    private EditText editText;
    private Button buttonStart;
    private Button buttonStop;
    private TextView textView;
    private TextView tvVersion;
    private boolean isRunning;
    private Handler handler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        editText = findViewById(R.id.editText);
        buttonStart = findViewById(R.id.buttonStart);
        buttonStop = findViewById(R.id.buttonStop);
        textView = findViewById(R.id.textView);

        tvVersion = findViewById(R.id.textViewVersion);
        tvVersion.setText(FFTools.version());

        FFTools.init();
        FFTools.setMsgListener((msg, level) -> postMsg(msg));

        isRunning = false;

        buttonStart.setOnClickListener(v -> {
            Intent intent = new Intent(this, PermissionsActivity.class);
            startActivity(intent);

            if (isRunning) {
                Toast toast = Toast.makeText(MainActivity.this,
                        "The ffmpeg program is busy,\nplease try again later.",
                        Toast.LENGTH_SHORT);
                toast.show();
                return;
            }

            if (PermissionsActivity.hasRequiredPermissions(this)) {
                AsyncHelper.doInBackground(() -> {
                    isRunning = true;
                    String strCmd = editText.getText().toString();
                    if (TextUtils.isEmpty(strCmd)) {
                        strCmd = "ffmpeg -h";
                        // strCmd = "ffmpeg -c:v libopenh264 -i /sdcard/video_h264.mp4 /sdcard/video_h264.yuv";
                    }
                    String[] commands = strCmd.split(" ");
                    FFTools.ffmpeg(commands);
                    isRunning = false;
                });
            }
        });

        buttonStop.setOnClickListener(v -> {
            Toast toast = Toast.makeText(MainActivity.this,
                    "Feature not available", Toast.LENGTH_SHORT);
            toast.show();
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        FFTools.deinit();
    }

    private void postMsg(String msg) {
        handler.post(() -> {
            String currentText = textView.getText().toString();
            textView.setText(String.format("%s\n%s", currentText, msg));

            textView.post(() -> {
                int scrollAmount = textView.getLayout().getLineTop(
                        textView.getLineCount()) - textView.getHeight();
                textView.scrollTo(0, Math.max(scrollAmount, 0));
            });
        });
    }
}
