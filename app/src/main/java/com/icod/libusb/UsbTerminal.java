package com.icod.libusb;
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//


import android.util.Log;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

public class UsbTerminal {
    static String TAG =UsbTerminal.class.getName();
    public static final String DEFAULTCMD="chmod -R 777 /dev/bus/usb/";
    public static final int SHCMD=0;
    public static final int SUCMD=1;

    public UsbTerminal() {
    }

    public static boolean exec(int typeCmd,String cmd) {
        Log.d(TAG, "^ Executing \'" + cmd + "\'");
        boolean success=false;
        Process e;
        try {
            if (SHCMD==typeCmd){
                e = Runtime.getRuntime().exec("sh");
            }else{
                e = Runtime.getRuntime().exec("su");
            }
            DataInputStream is = new DataInputStream(e.getInputStream());
            DataOutputStream os = new DataOutputStream(e.getOutputStream());
            os.writeBytes(cmd + "\n");
            os.writeBytes("exit\n");
            os.flush();
            os.close();
            BufferedReader reader = new BufferedReader(new InputStreamReader(is));

            try {
                String e1;
                String line1;
                for(e1 = ""; reader.readLine() != null; e1 = e1 + line1 + "\n") {
                    line1 = "";
                }

                success=true;
            } catch (IOException var9) {
                Log.e(TAG, "^ exec, IOException 1");
                var9.printStackTrace();
                e.waitFor();
                success=false;
            }
        } catch (IOException var10) {
            Log.e(TAG, "^ exec, IOException 2");
            var10.printStackTrace();
            success= false;
        } catch (InterruptedException var11) {
            Log.e(TAG, "^ exec, InterruptedException");
            var11.printStackTrace();
            success =false;
        }

        return success;
    }

   public static int permision(String path){
       File device=new File(path);
       if (!device.canRead() || !device.canWrite()) {
           try {
                /* Missing read/write permission, trying to chmod the file */
               Process su;
               //su = Runtime.getRuntime().exec("/system/bin/su");
               su = Runtime.getRuntime().exec("su"); //swm 获取root权限
               String cmd = "mount -o remount,rw " + "/dev/bus/usb" + " \n"
                       + "exit\n";
               su.getOutputStream().write(cmd.getBytes());
               if ((su.waitFor() != 0) || !device.canRead()
                       || !device.canWrite()) {
                   throw new SecurityException();
               }
           } catch (Exception e) {
               e.printStackTrace();
               throw new SecurityException();
           }
       }
       return 0;
   }

}
