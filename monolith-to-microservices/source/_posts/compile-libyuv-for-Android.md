---
title: Android环境下编译libyuv
reward: false
top: false
date: 2020-03-30 11:50:33
categories: Android
tags: 
  - libyuv
---

libyuv是Google开源的实现各种YUV格式与RGB格式之间相互转换、旋转、缩放的库。关于libyuv的具体介绍可以参考[官网介绍](https://chromium.googlesource.com/libyuv/libyuv)。

<!--more-->

## Android Studio编译libyuv
在自己编译libyuv的过程中，遇到了很多坑。在解决这些坑的过程中发现，从网上查询的很多资料并没有对问题的解决提供帮助。因此，在自己趟过了所有的坑之后，就想把编译libyuv的过程整理了出来，一方面是做一个梳理和总结，另一方面也希望帮助更多有类似需求的人。

接下来，开始介绍在Android平台上，利用NKD编译包含libjpeg库的libyuv库。这个主要通过[libyuv/Android.mk](https://chromium.googlesource.com/libyuv/libyuv/+/refs/heads/master/Android.mk)中的如下代码实现：

```
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := libyuv_static
LOCAL_MODULE := libyuv
ifneq ($(LIBYUV_DISABLE_JPEG), "yes")
LOCAL_SHARED_LIBRARIES := jpeg
endif

include $(BUILD_SHARED_LIBRARY)
```

## 需要下载的库
* [libyuv](https://chromium.googlesource.com/libyuv/libyuv)或者[libyuv](https//github.com/lemenkov/libyuv.git)
* [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo.git)

## 编译依赖
### NDK
因为NDK-r17之后的版本，不支持gcc编译so，需要使用clang来编译so库。因此编译时，需要确定好自己的NDK版本。

### CMAKE, NASM, GCC等
该部分编译依赖为编译libjpeg-turbo库的编译依赖，具体可以参见libjpeg-turbo的[BUILD.md](https://github.com/libjpeg-turbo/libjpeg-turbo/blob/master/BUILDING.md)

## 编译libjpeg-turbo
1. 下载编译脚本，编译脚本位于[libjpeg-turbo](https://github.com/wangwei1237/libyuv-with-jpeg/tree/master/libjpeg-turbo)目录下，共计有三个脚本：
    1. config.sh：配置编译参数，例如ANDROID_NDK_ROOT等，需要根据自己的环境变量进行替换。
    2. build_jpeg.sh：编译某个CPU架构的so库，该脚本中的变量一般不需要修改。
    3. build_jpeg_all.sh：编译所有CPU架构的so库

2. 下载[libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo)源码：`git clone https://github.com/libjpeg-turbo/libjpeg-turbo.git`。

3. 将第*1*步中下载到的三个文件复制到libjepg-turob源码的根目录下：`cp -r libjpeg-turbo/* JPEG_SRC_ROOT_PATH/`。

4. 修改*config.sh*中的`ANDROID_NDK_ROOT`为自己的NDK路径，修改`ANDROID_NDK_VERSION`为对应的数字版本。例如*r16b*版本的为`ANDROID_NDK_VERSION=16`。

5. 运行`sh build_jpeg_all.sh`编译libjpeg-turbo，运行之后会在当前目录的*libs*目录下生成各种CPU架构对应的so库。如下图所示：

    ![图1. libjpeg-turbo编译产物](1.jpg)

## 编译libyuv
1. 将之前下载的libyuv的源码导入到Android Studio中的指定目录：PROJECT/app/jni/libyuv,该目录自己定义就可以。

2. 将上一步生成的so文件按照ABI的格式复制到libyuv/libs目录下，如下图所示：

    ![图2](2.png)

3. 修改libyuv目录下的Android.mk文件，[libyuv/Android.mk](https://github.com/wangwei1237/libyuv-with-jpeg/blob/master/myapp/jni/libyuv/Android.mk)文件中的第4~12行的内容即可。

    ```
    ########################################################
    ## {{BEGIN 增加如下的代码
    include $(CLEAR_VARS)
    LOCAL_MODULE := jpeg
    LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libjpeg.so
    LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
    include $(PREBUILT_SHARED_LIBRARY)
    ## END}}
    ########################################################
    ```

4. 按照[仓库](https://github.com/wangwei1237/libyuv-with-jpeg)所示的[myapp/jni](https://github.com/wangwei1237/libyuv-with-jpeg/tree/master/myapp/jni)目录下的*Android.mk*和*Application.mk*文件修改自己代码中的对应文件即可，主要是用于编译libyuv使用。

5. 修改自己app目录下的[build.gradle](https://github.com/wangwei1237/libyuv-with-jpeg/blob/master/myapp/build.gradle)，在android{}中增加如下编译任务

    ```
    externalNativeBuild {
        ndkBuild {
            path file('jni/Android.mk')
        }
    }
    ```

6. 点击Android Studio的编译按钮编译即可，编译后生成的编译产物如下图所示：

    ![图3. libyuv的编译产物](3.jpg)