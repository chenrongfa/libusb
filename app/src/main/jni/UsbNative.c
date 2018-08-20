//
// Created by Administrator on 2018/7/4.
//
#include "UsbNative.h"

#include <sys/wait.h>
#include <libusb/libusbi.h>
#include "android/log.h"
#define STD_USB_REQUEST_GET_DESCRIPTOR  0x06
static const char *TAG = "usb";
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO,  TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)
struct device_descript{
    libusb_device_handle *handler;
    libusb_device    *dev;
    int  out;
    int  in;
    int outTime;
    int inTime;
    int vendorId;
    int productId;

};
int dev_num;
void throwException(JNIEnv *env, const char *className, const char *mess){
    jclass exceClas= (*env)->FindClass(env,className);
    (*env)->ThrowNew(env,exceClas,mess);
    (*env)->DeleteLocalRef(env,exceClas);
}
int checkNULL(JNIEnv const *env,const char *log) {
    if ((*env)->ExceptionOccurred(env)!=NULL){
        (*env)->ExceptionClear(env);
        LOGI("usb%s",log);
        throwException(env,"java/lang/NullPointerException","空指针或者无法分配内存");
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

struct device_descript dev_list[DEV_MIN];
jstring  getString2(JNIEnv *env,jbyteArray array){
   if ((*env)->PushLocalFrame(env,3)!=JNI_OK){
        return NULL;
    }
  jclass stringCla= (*env)->FindClass(env,"java/lang/String");
    if(stringCla==NULL){
       // (*env)->PopLocalFrame(env,NULL);
        throwException(env,"java/lang/NullPointerException","空指针或者无法分配内存");
    }
    jmethodID  jmethodID1=(*env)->GetMethodID(env,stringCla,"<init>","([BLjava/lang/String;)V");
    if (jmethodID1==NULL){
       (*env)->PopLocalFrame(env,NULL);
        throwException(env,"java/lang/NullPointerException","空指针或者无法分配内存");
    }
    jobject  jobject1=(*env)->NewObject(env,stringCla,jmethodID1,array,(*env)->NewStringUTF(env,"utf-8"));
  jobject stringObject=(*env)->PopLocalFrame(env,jobject1);

   return stringObject;

}
jstring  getStringByChar(JNIEnv *env,unsigned char  array[]){
   int size= strlen(array);
    if ((*env)->PushLocalFrame(env,size)!=JNI_OK){
        return NULL;
    }
   jbyteArray data=(*env)->NewByteArray(env,size);
    if(data==NULL){
        (*env)->PopLocalFrame(env,NULL);
        throwException(env,"java/lang/NullPointerException","空指针或者无法分配内存");
    }
    (*env)->SetByteArrayRegion(env,data,0,size,array);
   jstring  tmpString=getString2(env,data);
    jstring backString=(*env)->PopLocalFrame(env,tmpString);
    return backString;

}


jstring
getString(JNIEnv const *env, const void *usbNativeCls, const struct _jmethodID *stringMethodId,
          int size, const unsigned char *dataChar){
    jbyteArray productByte=(*env)->NewByteArray(env,size);
    (*env)->SetByteArrayRegion(env,productByte,0,size,dataChar);
    jstring productString=(*env)->CallStaticObjectMethod(env,usbNativeCls,stringMethodId,
                                                            productByte,(*env)->NewStringUTF(env, "utf-8"));
    return productString;
}

libusb_context *ctx = NULL;
jfieldID getFieldByName(JNIEnv *env, jclass  clss,const char * methodNmae,const char
*sig){
    jfieldID fieldId= (*env)->GetFieldID(env,clss,methodNmae,sig);
    if (fieldId==NULL){
        /*jclass exceClas= (*env)->FindClass(env,"java/lang/NullPointerException");
        (*env)->ThrowNew(env,exceClas,"空指针或者无法分配内存");
        (*env)->DeleteLocalRef(env,exceClas);*/
        throwException(env,"java/lang/NullPointerException","空指针或者无法分配内存");
    }
    return fieldId;
}

const char * getDesc(int size, unsigned char data[],char getData[]){
    if (size<=0){
        const char* unKnow_device="unknow_device";

        return unKnow_device;
    }
    const int constSize=size;

    int i=0;
    for(;i<constSize;i++){
        getData[i]=data[i];
    }
    memset(data,0,strlen(data));
    return getData;
}
void dev_find_endpoints(libusb_device *dev, int vendorId, int productId) {
    struct	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
        LOGI("tag %s","failed to get device descriptor");
		return;
	}

    LOGI("Number of possible configurations: %d\n",
		(int)desc.bNumConfigurations);
    LOGI("Device Class: %d\n", (int)desc.bDeviceClass);
    LOGI("VendorID:%d \n", desc.idVendor);
    LOGI("ProductID: %d\n", desc.idProduct);
    struct libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	//printf("Interfaces: %d", (int)config->bNumInterfaces);
    	const struct libusb_interface *inter;
	const struct libusb_interface_descriptor *interdesc;
	const struct libusb_endpoint_descriptor *epdesc;
	if (desc.idVendor == vendorId && desc.idProduct == productId) {
		dev_list[dev_num].dev = dev;
		dev_list[dev_num].vendorId = desc.idVendor;
		dev_list[dev_num].productId = desc.idProduct;
		for (int i = 0; i < (int)config->bNumInterfaces; i++) {
			inter = &config->interface[i];
			LOGI("Number of alternate settings: %d\n", inter->num_altsetting);
			for (int j = 0; j < inter->num_altsetting; j++) {
				interdesc = &inter->altsetting[j];
				LOGI("Interface Number: %d\n", (int)interdesc->bInterfaceNumber);
				LOGI("Interface class: %d\n", (int)interdesc->bInterfaceClass);
				LOGI("Number of endpoints: %d\n", (int)interdesc->bNumEndpoints);
				if (interdesc->bInterfaceClass == LIBUSB_CLASS_PRINTER) {
					for (int k = 0; k < (int)interdesc->bNumEndpoints; k++) {
						epdesc = &interdesc->endpoint[k];
						LOGI("Descriptor Type:%d \n", (int)epdesc->bDescriptorType);
						LOGI("EP Address: %d\n", (int)epdesc->bEndpointAddress);
						int address = epdesc->bEndpointAddress;
						if (address >= 128) {
							dev_list[dev_num].in = address;
						}
						else {
							dev_list[dev_num].out = address;
						}
					}
				}
			}
		}
		dev_num++;
	}
}
JNIEXPORT jint JNICALL Java_com_icod_libusb_UsbNative_devFind
  (JNIEnv *env, jobject obj, jint vendor_id, jint product_id){
  libusb_device **devs;

  	libusb_init(&ctx);
  	int cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
  	if (cnt < 0) {
  		LOGI("Get Device Error \n");

  		return 1;
  	}
  	LOGI("Devices in list. %d \n", cnt);

  	int i = 0;
  	for (i = 0; i < cnt; i++) {

  		dev_find_endpoints(devs[i], vendor_id, product_id);
  	}
  	libusb_free_device_list(devs, 1);
  	return dev_num;

  }
JNIEXPORT jstring JNICALL Java_com_icod_libusb_UsbNative_permissionPath(JNIEnv *env, jobject obj,jint dev_no) {
    libusb_device *pDevice=dev_list[dev_no].dev;
  uint8_t  bus= pDevice->bus_number;
  uint8_t  address= pDevice->device_address;
    LOGE("nihao %d",bus);
    LOGE("nihao %d",address);
   char path[PATH_MAX];
    const char *usbfs_path="/dev/bus/usb";
    snprintf(path, PATH_MAX, "%s/%03d",
             usbfs_path, pDevice->bus_number, pDevice->device_address);

    return (*env)->NewStringUTF(env,path);

}
/*
 * Class:     com_icod_libusb_UsbNative
 * Method:    devOpen
 * Signature: (I)I
 */
JNIEXPORT  jint JNICALL Java_com_icod_libusb_UsbNative_devOpen
  (JNIEnv *env, jobject obj, jint dev_no){

  dev_num = dev_no;
  	if (NULL != dev_list[dev_num].dev) {
      //  permissionPath(dev_list[dev_num].dev);
  		libusb_device_handle *handler=libusb_open_device_with_vid_pid(ctx,dev_list[dev_num].vendorId,dev_list[dev_num].productId);
  		if (handler ==NULL) {
  			LOGI("open error \n" );
  			return -1;
  		}

  		dev_list[dev_num].handler = handler;
  		if (libusb_kernel_driver_active(dev_list[dev_no].handler, 0) == 1) { //find out if kernel driver is attached
  			LOGI("Kernel Driver Active.");

  			if (libusb_detach_kernel_driver(dev_list[dev_no].handler, 0) == 0) //detach it
  				LOGI("Kernel Driver Detached!");

  		}
  		int r = libusb_claim_interface(dev_list[dev_no].handler, 0); //claim interface 0 (the first) of device (mine had jsut 1)
  		if (r < 0) {
  			LOGI("Cannot Claim Interface\n");

  			return -1;
  		}
  		LOGI("Claimed Interface \n");
  	}else{
        return -1;
    }
  	return 0;

  }

/*
 * Class:     com_icod_libusb_UsbNative
 * Method:    devWrite
 * Signature: (I[BI)I
 */
 jint  Java_com_icod_libusb_UsbNative_devWrite
  (JNIEnv *env, jobject this, jint dev_no, jbyteArray data, jint lenght){
    jboolean  copy;
    jbyte *pData= (*env)->GetByteArrayElements(env,data,&copy);
    jint len_size;
    jint size = libusb_bulk_transfer(dev_list[dev_no].handler, (dev_list[dev_no].out |
                                             LIBUSB_ENDPOINT_OUT),
                                     pData, lenght, &len_size,
                                    dev_list[dev_no].outTime);
    (*env)->ReleaseByteArrayElements(env,data,pData,0);

#ifdef DEBUG

    LOGI("Out length: %d;Success length: %d\n", lenght, len_size);
#endif


    if (len_size == lenght && size == 0) {
        return len_size;
    }


    return size;

}

/*
 * Class:     com_icod_libusb_UsbNative
 * Method:    devRead
 * Signature: (I[BI)I
 */
JNIEXPORT jint JNICALL Java_com_icod_libusb_UsbNative_devRead
  (JNIEnv *env, jobject obj, jint dev_no, jbyteArray data, jint lenght){

  int recv_len;
  	int size = 0;
    jboolean  copy;
   jbyte *pData= (*env)->GetByteArrayElements(env,data,&copy);
  	size = libusb_bulk_transfer(dev_list[dev_no].handler, (dev_list[dev_no].in |
                                        LIBUSB_ENDPOINT_IN), pData, lenght,
  		&recv_len, dev_list[dev_no].inTime);
    (*env)->ReleaseByteArrayElements(env,data,pData,0);


  #ifdef DEBUG
  	LOGI("\nReceived data length: %d\n", recv_len);
  #endif

  	if (recv_len < 0) {
  		return RD_ER;
  	}
  	return recv_len;
  }
  int dev_isOpen(int dev_no) {
  	if (dev_list[dev_no].handler != NULL) {
  		return 0;
  	}
  	return 1;

  }

/*
 * Class:     com_icod_libusb_UsbNative
 * Method:    devClose
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_icod_libusb_UsbNative_devClose
  (JNIEnv *env, jobject obj, jint dev_no){
   	jint result = libusb_release_interface(dev_list[dev_no].handler, 0);
   	libusb_close(dev_list[dev_no].handler); //close the device we opened
   	dev_list[dev_no].handler = NULL;
   	libusb_exit(ctx);
   	ctx = NULL;
   	return result;


  }

/*
 * Class:     com_icod_libusb_UsbNative
 * Method:    devIsOpen
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_icod_libusb_UsbNative_devIsOpen
  (JNIEnv *env, jobject obj, jint dev_no){

	if (dev_list[dev_no].handler != NULL) {
		return 0;
	}
	return 1;

  }

/*
 * Class:     com_icod_libusb_UsbNative
 * Method:    devIsOnline
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_icod_libusb_UsbNative_devIsOnline
  (JNIEnv *env, jobject obj, jint dev_no){
  unsigned char data[] = "\x10\x04\x04";
  	LOGI("data length %d", sizeof(data) / sizeof(char));
  	jint dataSend = Java_com_icod_libusb_UsbNative_devWrite(env, obj,dev_no, data, sizeof
(data) / sizeof
                                                                                               (char));

  	LOGI("sended size %d", dataSend);
  	unsigned char reciever[1];

  	int rev = Java_com_icod_libusb_UsbNative_devRead(env,obj,dev_no, reciever, 1);
  	LOGI("reviecer %d", rev);
  	jint i = 0;
  	for (; i < rev; i++) {
  		LOGI("data reviecer %d", reciever[i]);
  	}

  	return i > 0 ? 0 : 1;

  }

/*
 * Class:     com_icod_libusb_UsbNative
 * Method:    dev_setOption
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_icod_libusb_UsbNative_devSetOption
  (JNIEnv *env, jobject obj, jint dev_no, jint outTime, jint inTime){
  	dev_list[dev_no].outTime = outTime;
  	dev_list[dev_no].inTime = inTime;
  }

JNIEXPORT jint JNICALL
Java_com_icod_libusb_UsbNative_system(JNIEnv *env, jobject instance, jstring cmd_) {
    const char *cmd = (*env)->GetStringUTFChars(env, cmd_, 0);
   jint success = system(cmd);

    (*env)->ReleaseStringUTFChars(env, cmd_, cmd);
    return success;
}
JNIEXPORT jbyteArray JNICALL
Java_com_icod_libusb_UsbNative_getProductName(JNIEnv *env, jobject instance, jint dev_no) {

    struct	libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev_list[dev_no].dev, &desc);
    if (r < 0) {
        LOGI("tag %s","failed to get device descriptor");
        return (*env)->NewStringUTF(env,"");
    }

    unsigned char data[255];
    unsigned char data2[255];
    int total=0;

    memset(data,0,255);
    memset(data2,0,255);
      int size=   libusb_get_string_descriptor_ascii(dev_list[dev_no].handler,desc.iProduct,
                                            data,
                                      255);
    total=total+size;
    for (int i = 0; i < size; ++i) {
        data2[i]=data[i];

    }
    memset(data,0,255);
   size= libusb_get_string_descriptor_ascii(dev_list[dev_no].handler,desc.iManufacturer,
                                       data,
                                       255);
    for (int i = 0; i < size; ++i) {
        data2[i+total]=data[i];

    }
    total+=size;
    LOGI("data len %d",strlen(data));
    LOGI("tag %s",data);
    LOGI("tag %d",size);
    if (total<=0){
        const char* unKnow_device="unknow_device";
        strcpy(data,unKnow_device);
        int len=strlen(data);
        jbyteArray productNmae=(*env)->NewByteArray(env,len);
        (*env)->SetByteArrayRegion(env,productNmae,0,len,data);
        return productNmae;
    }
    jbyteArray productNmae=(*env)->NewByteArray(env,total);
    (*env)->SetByteArrayRegion(env,productNmae,0,total,data2);
    return  productNmae;
}

JNIEXPORT  jobject JNICALL
Java_com_icod_libusb_UsbNative_getConnectedDesc(JNIEnv *env, jobject instance, jint dev_no) {


    (*env)->PushLocalFrame(env,16);
    if(checkNULL(env,"无法创建这个数量的局部引用"))return NULL;
    jclass clsUsb=NULL;
    clsUsb=(*env)->FindClass(env,"com/icod/libusb/UsbIcodDevice");
    if(checkNULL(env,"无法创建UsbIcodDevice"))return NULL;

    jmethodID  usbMethodId=(*env)->GetMethodID(env,clsUsb,"<init>","()V");
    if(checkNULL(env,"找不到构造方法"))return NULL;
   jclass usbNativeCls=(*env)->GetObjectClass(env,instance);
        jmethodID stringMethodId=(*env)->GetStaticMethodID(env,usbNativeCls,"createString",""
                "([BLjava/lang/String;)Ljava/lang/String;");
    if(checkNULL(env,"找不到createString"))return NULL;
    jobject  usbObj=(*env)->NewObject(env,clsUsb,usbMethodId);
    if(checkNULL(env,"无法创建UsbIcodDevice对象"))return NULL;


    jfieldID produceField=getFieldByName(env,clsUsb,"mProductName","Ljava/lang/String;");
    jfieldID manifacturerField=getFieldByName(env,clsUsb,"mManufacturer","Ljava/lang/String;");
    jfieldID serialNumberField=getFieldByName(env,clsUsb,"mSerialNumber","Ljava/lang/String;");
    jfieldID vendorField=getFieldByName(env,clsUsb,"mVendorId","I");
    jfieldID productIdField=getFieldByName(env,clsUsb,"mProductId","I");
    jfieldID classField=getFieldByName(env,clsUsb,"mClass","I");
    jfieldID protocolField=getFieldByName(env,clsUsb,"mProtocol","I");
    jfieldID maxSizeField=getFieldByName(env,clsUsb,"maxSize","I");

    int size=0;
    unsigned char data[255];
    memset(data,0,255);
    struct	libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev_list[dev_no].dev, &desc);
    if (r < 0) {
        LOGI("tag %s","failed to get device descriptor");
        return NULL;
    }
    size=libusb_get_string_descriptor_ascii(dev_list[dev_no].handler,desc.iManufacturer,data,255);
   unsigned char manifacturerChar[size];
    getDesc(size,data,manifacturerChar);
    jstring manifactuerString= getString(env, usbNativeCls, stringMethodId, size,
                                         manifacturerChar);
    size=libusb_get_string_descriptor_ascii(dev_list[dev_no].handler, desc.iProduct, data, 255);
    unsigned char productChar[size];
    getDesc(size,data,productChar);

    jstring productString = getString(env, usbNativeCls, stringMethodId, size,
                                           productChar);

    size = libusb_get_string_descriptor_ascii(dev_list[dev_no].handler, desc.iSerialNumber, data,
                                              255);
    unsigned  char serialNumberChar[size];
     getDesc(size,data,serialNumberChar);
    jstring serialNumberString = getStringByChar(env,serialNumberChar);
    (*env)->SetIntField(env,usbObj,maxSizeField,desc.bMaxPacketSize0);
    (*env)->SetIntField(env,usbObj,protocolField,desc.bDeviceProtocol);
    (*env)->SetIntField(env,usbObj,classField,desc.bDeviceClass);
    (*env)->SetIntField(env,usbObj,vendorField,desc.idVendor);
    (*env)->SetIntField(env,usbObj,productIdField,desc.idProduct);
    (*env)->SetObjectField(env,usbObj,produceField,productString);
    (*env)->SetObjectField(env,usbObj,manifacturerField,manifactuerString);
    (*env)->SetObjectField(env,usbObj,serialNumberField,serialNumberString);
    (*env)->DeleteLocalRef(env,usbNativeCls);
  jobject backObj= (*env)->PopLocalFrame(env,usbObj);

    if(checkNULL(env,"释放失败"))return NULL;
    return backObj;
}
