package com.icod.libusb;

import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

import static com.icod.libusb.UsbTerminal.TAG;

/**
 * Created by crf on 2018/7/4.
 */

public class UsbNative {
    private int dev_no = -1;

    public int getDev_no() {
        return dev_no - 1;
    }

    public void setDevNumber(int dev_no) {
        this.dev_no = dev_no;
    }

    public native int devFind(int vendorId, int productId);

    public native int devOpen(int dev_no);

    /**
     * \brief Write data to one target USB device.
     */
    public native int devWrite(int dev_no, byte data[], int lenght);

    public native int system(String cmd);

    /**
     * \brief Read data from one target USB device.
     */
    public native int devRead(int dev_no, byte data[], int lenght);

    /**
     * \brief Close one target USB device.
     */
    public native int devClose(int dev_no);

    public native int devIsOpen(int dev_no);

    public native void devSetOption(int dev_no, int outTime, int inTime);

    public native String permissionPath(int dev_no);

    public native byte[] getProductName(int dev_no);

    public native UsbIcodDevice getConnectedDesc(int dev_no);

    public native int devIsOnline(int dev_no);

    public static String createString(byte data[], String charset) {
        try {
            //return new String(productName,2,productName.length-2,"UTF-16LE");
            Log.i(TAG, "createString: " + Arrays.toString(data) + " " + charset);
            return new String(data, 0, data.length, charset).trim();
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return "";
    }
}
