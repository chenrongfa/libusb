# Android build config for libusb
# Copyright © 2012-2013 RealVNC Ltd. <toby.gray@realvnc.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
LOCAL_PATH:= $(call my-dir)
# libusb
include $(CLEAR_VARS)
LOCAL_SRC_FILES := UsbNative.c\
libusb/core.c \
  libusb/descriptor.c \
  libusb/hotplug.c \
  libusb/io.c \
  libusb/sync.c \
  libusb/strerror.c \
  libusb/os/linux_usbfs.c \
  libusb/os/poll_posix.c \
  libusb/os/threads_posix.c \
  libusb/os/linux_netlink.c
LOCAL_LDLIBS := -llog
LOCAL_MODULE := libusb1.0
include $(BUILD_SHARED_LIBRARY)
