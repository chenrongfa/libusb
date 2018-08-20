package com.icod.libusb;


import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

import static android.content.ContentValues.TAG;

public class MainActivity extends Activity implements View.OnClickListener {
   static {
       System.loadLibrary("usb1.0");
   }
    UsbNative usbNative;
    UsbConnection usbConnection;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        findDevice();
        findViewById(R.id.btn_open).setOnClickListener(this);
        findViewById(R.id.btn_read).setOnClickListener(this);
        findViewById(R.id.btn_write).setOnClickListener(this);
    }

    private void findDevice() {
        usbNative=new UsbNative();
        usbConnection=new UsbConnection(usbNative);
        int i = usbConnection.find();
        if (i>0){
            usbNative.setDevNumber(i);
            Log.i(TAG, "onCreate: 成功");
        }
    }

    @Override
    public void onClick(View v) {
        if (usbNative.getDev_no()==-1){
            Log.i(TAG, "onClick: 没有任何发现请重新 find");
            findDevice();
            return;
        }
        switch (v.getId()){
            case R.id.btn_open:
                if (!UsbTerminal.exec(UsbTerminal.SUCMD, UsbTerminal.DEFAULTCMD)){
                    UsbTerminal.exec(UsbTerminal.SHCMD,UsbTerminal.DEFAULTCMD)  ;

                }
                String s = usbNative.permissionPath(usbNative.getDev_no());
                Log.i(TAG, "onClick: "+s);
                UsbTerminal.exec(UsbTerminal.SUCMD,UsbTerminal.DEFAULTCMD);
                int open = usbConnection.open(usbNative.getDev_no());
                Log.i(TAG,"open"+open);
                if (open==0){
                    Log.i(TAG, "onClick: open 成功");
                }else{
                    Log.i(TAG, "onClick:open 失败 ");
                }
            break;
            case R.id.btn_write:
                try {
                    UsbIcodDevice s1 = usbConnection.getConnectedDesc();
                    if (s1!=null)
                        Log.d(TAG,s1.toString());
                    else{
                        Log.i(TAG, "onClick: ");
                    }
                }catch (Exception e){
                    Log.i(TAG, "onClick: "+e.getMessage());
                }


                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String trim = usbConnection.getProductName(usbNative.getDev_no()).trim()
                                .replaceAll(" ","");
                        String[] split = trim.split(" ");
                        Log.d(TAG,Arrays.toString(split));

                        byte data[]=null;
                        try {
                            data="中国人民共和国".getBytes("gbk");
                        } catch (UnsupportedEncodingException e) {
                            e.printStackTrace();
                        }
                        int writeSize = usbConnection.write(usbNative.getDev_no(),data,data.length);
                        if (writeSize==data.length){
                            Log.i(TAG, "onClick: writeSize 成功");
                        }
                    }
                }).start();

            break;
            case R.id.btn_read:
                byte data1[]={16,4,1};
                int writeSize2 = usbConnection.write(usbNative.getDev_no(),data1,data1.length);
                if (writeSize2==data1.length){
                    byte readData[]=new byte[10];
                    Log.i(TAG, "onClick: writeSize 成功");
                    int readSize = usbConnection.read(usbNative.getDev_no(),readData,1);
                    if (readSize > 0) {
                        Log.i(TAG, "onClick: readSize 成功"+ Arrays.toString(readData));
                    }
                }
                break;
            case R.id.btn_close:
             if (usbConnection.isOpen(usbNative.getDev_no())==0){
                 usbConnection.close(usbNative.getDev_no());
             }else{
                 Log.i(TAG, "onClick: "+"没有打开usb");
             }
                break;

        }
    }
}
