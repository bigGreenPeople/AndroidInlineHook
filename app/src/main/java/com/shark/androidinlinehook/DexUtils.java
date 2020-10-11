package com.shark.androidinlinehook;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.lang.reflect.Method;

import dalvik.system.DexClassLoader;

public class DexUtils {

    public static final String SHARK = "shark";

    public static void exeCoreMethod(Context context) {
        try {
            //创建文件夹
            File optfile = context.getDir("opt_dex", 0);
            File libfile = context.getDir("lib_path", 0);
            //得到当前Activity 的ClassLoader 以下的方法得到的都是同一个ClassLoader
            ClassLoader parentClassloader = MainActivity.class.getClassLoader();
            ClassLoader tmpClassLoader = context.getClassLoader();
            //创建我们自己的DexClassLoader 指定其父节点为当前Activity 的ClassLoader
            /*dexPath:目标所在的apk或者jar文件的路径，装载器将从路径中寻找指定的目标类。
            dexOutputDir:由于dex 文件在APK或者 jar文件中，所以在装载前面前先要从里面解压出dex文件，这个路径就是dex文件存放的路径，
            在 android系统中，一个应用程序对应一个linux用户id ,应用程序只对自己的数据目录有写的权限，所以我们存放在这个路径中。
            libPath :目标类中使用的C/C++库。
        最后一个参数是该装载器的父装载器，一般为当前执行类的装载器。*/
            DexClassLoader dexClassLoader = new DexClassLoader("/sdcard/CoreDex.dex",
                    optfile.getAbsolutePath(), libfile.getAbsolutePath(), MainActivity.class.getClassLoader());

            Class<?> clazz=dexClassLoader.loadClass("com.shark.calculate.CoreUtils");
//            Method calculateMoney=clazz.getDeclaredMethod("calculateMoney",int.class,int.class);
//            Object obj=clazz.newInstance();
//            int result = (int)calculateMoney.invoke(obj,2,3);
//            Log.i(SHARK, "calculateMoney result:" + result);
            //------------------------------------------------------------------------
            //getPwd方法执行
            Method getPwd=clazz.getDeclaredMethod("getPwd");
            Log.i(SHARK, "getPwd result:" + getPwd.invoke(null));

        } catch (Exception e) {
            Log.i(SHARK, "exec exeCoreMethod err:" + Log.getStackTraceString(e));
        }
    }
}
