package com.icod.libusb;

import java.io.Serializable;

/**
 * Created by crf on 2018/8/7.
 */

public class UsbIcodDevice implements Serializable{
        private  String mProductName;
        private  String mManufacturer;
        private  String mSerialNumber;
        private  int mVendorId;  //
        private int mProductId; //生产id
        private  int mClass;//7 代表打印类
        private  int mProtocol;//协议
        private  int maxSize;//最大一次能发送多少个字节


 private UsbIcodDevice(){}
    public String getmProductName() {
        return mProductName;
    }

    public String getmManufacturer() {
        return mManufacturer;
    }

    public String getmSerialNumber() {
        return mSerialNumber;
    }

    public int getmVendorId() {
        return mVendorId;
    }

    public int getmProductId() {
        return mProductId;
    }

    public int getmClass() {
        return mClass;
    }

    public int getmProtocol() {
        return mProtocol;
    }

    public int getMaxSize() {
        return maxSize;
    }

    @Override
    public String toString() {
        return "UsbIcodDevice{" +
                "mProductName='" + mProductName + '\'' +
                ", mManufacturer='" + mManufacturer + '\'' +
                ", mSerialNumber='" + mSerialNumber + '\'' +
                ", mVendorId=" + mVendorId +
                ", mProductId=" + mProductId +
                ", mClass=" + mClass +
                ", mProtocol=" + mProtocol +
                ", maxSize=" + maxSize +
                '}';
    }
}
