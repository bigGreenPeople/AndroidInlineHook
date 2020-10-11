package com.shark.androidinlinehook;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("inlineHook");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //设置点击事件 加载调用Dex中的计算方法
        findViewById(R.id.button1).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                DexUtils.exeCoreMethod(getApplicationContext());
            }
        });
        startInlineHook();
    }

    //开启hook方法
    public static native void startInlineHook();
}
