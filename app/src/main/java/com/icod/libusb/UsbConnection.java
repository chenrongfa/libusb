package com.icod.libusb;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Iterator;

import static android.content.ContentValues.TAG;

/**
 * Created by crf on 2018/7/5.
 */

public class UsbConnection {
    private UsbNative mUsbNative;
    private boolean isPermission=false;
    public UsbIcodDevice getUsbIcodDevice() {
        return usbIcodDevice;
    }

    private UsbIcodDevice usbIcodDevice;
    public UsbConnection(UsbNative usbNative) {
        mUsbNative = usbNative;
    }
    public int find() {
        for (int i = 0; i < lenght; i++) {
            int success = mUsbNative.devFind(mUsbCheckSRT[i].VID, mUsbCheckSRT[i].PID);
            if (success > 0) {
                Log.i(TAG, "open: 已经找到" + mUsbCheckSRT[i].VID + "   " + mUsbCheckSRT[i].PID);
                return success;
            }
        }

        return -1;
    }

    public int open(int dev_no) {
        int result=-1;
        if (mUsbNative.devOpen(dev_no)==0){
            usbIcodDevice=getConnectedDesc();
            result=0;
        }
        return result;
    }
    public int close(int dev_no) {
        int close = mUsbNative.devClose(dev_no);
        if (close==0){
            mUsbNative=null;
            return 0;
        }
        return -1;
    }

    public int write(int dev_no, byte data[], int size) {
        return mUsbNative.devWrite(dev_no, data, size);
    }

    public int read(int dev_no, byte data[], int size) {
        return mUsbNative.devRead(dev_no, data, size);
    }
    public String getProductName(int  dev_no){
        byte[] productName = mUsbNative.getProductName(dev_no);
        try {
            //return new String(productName,2,productName.length-2,"UTF-16LE");
            return new String(productName,0,productName.length,"utf-8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return "";
    }
    public int isOpen(int dev_no) {
        return mUsbNative.devIsOpen(dev_no);
    }

    public int isOnLine(int dev_no) {
        return mUsbNative.devIsOnline(dev_no);
    }

    private static UsbCheckSRT[] mUsbCheckSRT = {

            new UsbCheckSRT(1157, 30017),
            new UsbCheckSRT(6790, 30084),
            new UsbCheckSRT(3544, 5120),
            new UsbCheckSRT(483, 7540),
            new UsbCheckSRT(8401, 28680),
            new UsbCheckSRT(1659, 8965),
            new UsbCheckSRT(1046, 20497),
            new UsbCheckSRT(7344, 3),
            new UsbCheckSRT(1155, 30016),
            new UsbCheckSRT(1155, 41064)

    };
    private static int lenght = mUsbCheckSRT.length;

    private static class UsbCheckSRT {
        public int PID = 0;
        public int VID = 0;

        public UsbCheckSRT(int vid, int pid) {
            this.PID = pid;
            this.VID = vid;
        }

        public boolean check(int vid, int pid) {
            if ((this.PID != pid) || (this.VID != vid)) {
                return false;
            }
            return true;
        }
    }

    /**
     *  现在貌似不需要
     * @param context
     * @return
     */
    public int usbPermission(Context context){
        UsbManager usbManager= (UsbManager) context.getSystemService(Context.USB_SERVICE);
        HashMap<String, UsbDevice> deviceList =usbManager
                .getDeviceList();
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        UsbDevice usbDevice=null;
        while (deviceIterator.hasNext()) {
           usbDevice = ((UsbDevice) deviceIterator.next());

            if (checkID(usbDevice.getVendorId(), usbDevice.getProductId())) {
                Log.i(TAG, "找到 匹配USB VID=" + usbDevice.getVendorId() + " PID="
                        + usbDevice.getProductId());
                break;
            }
            usbDevice = null;
        }
        if (usbDevice == null) {
            Log.i(TAG," 找不到 匹配USB VID 和PID" );
            return -1;
        }
        PendingIntent permissionIntent = PendingIntent.getBroadcast(context, 0,
                new Intent(ACTION_USB_PERMISSION), 0);

        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        context.registerReceiver(this.mUsbReceiver, filter);

        if (!usbManager.hasPermission(usbDevice)) {
            isPermission = false;
            usbManager.requestPermission(usbDevice,
                    permissionIntent);
            int num=0;
            while (!isPermission) {
                try {

                    Thread.sleep(50l);
                    if (20==num){
                        return -3;
                    }
                    if (num%2==0){
                        context.unregisterReceiver(mUsbReceiver);
                        context.registerReceiver(mUsbReceiver,filter);
                    }
                    num++;
                    Log.i(TAG,"usb 获取权限中....");
                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }

        }
        context.unregisterReceiver(mUsbReceiver);
        return isPermission?0:-1;
    }
    private static final String ACTION_USB_PERMISSION = "com.szsicod.print.USB_PERMISSION";
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {

                    UsbDevice device = (UsbDevice) intent
                            .getParcelableExtra("device");
                    if ((intent.getBooleanExtra("permission", false))
                            && (device != null)) {
                        Log.i(TAG,"usb 设备已经授权 ");
                        isPermission = true;
                    } else {
                        isPermission = false;
                        Log.i(TAG,"usb 设备没有授权 ");
                    }
                }
            }


        }
    };
    private boolean checkID(int pid, int vid) {
        for (int n = 0; n < this.mUsbCheckSRT.length; n++) {
            if (this.mUsbCheckSRT[n].check(pid, vid)) {
                return true;
            }
        }
        return false;
    }
    public UsbIcodDevice getConnectedDesc(){

        return mUsbNative.getConnectedDesc(mUsbNative.getDev_no());
    }
}
