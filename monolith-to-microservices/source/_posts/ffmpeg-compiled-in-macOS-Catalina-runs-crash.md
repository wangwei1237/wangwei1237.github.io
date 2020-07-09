---
title: macOS Catalina的strip版本异常导致编译的FFMpeg运行被kill
reward: false
top: false
date: 2020-04-08 09:23:19
categories: 
  - 视频技术
tags:
  - FFMpeg
  - 编译
  - macOS Catalina
  - strip
---

因为近期正在调研&了解[SRT协议](https://github.com/Haivision/srt)的相关内容，为了方便，需要打开FFMpeg的`--enable-libsrt`功能并重新编译FFMpeg，从而保证FFMpeg支持SRT协议。但是重新编译之后却发现启动ffmpeg工具就会被内核杀死，具体如下所示。

![](1.jpg)
<!--more-->

利用*lldb*调试该*ffmpeg*发现直接提示`error: Malformed Mach-o file`的错误，具体如下：

![](2.jpg)

编译异常的Mac版本和Xcode版本信息分别如下：

![](3.jpg)

查阅了[网上的资料](https://trac.ffmpeg.org/ticket/8073)，也查看了[brew编译FFMpeg的指令](https://github.com/Homebrew/homebrew-core/blob/master/Formula/ffmpeg.rb)，原因可能是：**Xcode11下clang默认开启-fstack-check**。

同时，在如下图所示的Xcode 10.3版本的Mac上则可编译成功：

![](4.jpg)

因此根据如上信息：`./configure`时需要增加`--host-cflags=-fno-stack-check`配置。

但是增加该配置之后，编译出的ffmpeg依然无法正常运行。

## 发现原因

**恰好最近升级了系统和Xcode版本，想着再编译一下看下是否可用，终于有了新的发现。**

编译的系统版本和Xcode版本如下所示：

![](5.jpg)

编译命令如下所示：

```
 ./configure --enable-shared --enable-pthreads --enable-version3 --enable-avresample \
--cc=clang --host-cflags= --host-ldflags= --enable-ffplay --enable-gnutls --enable-gpl \
--enable-libaom --enable-libbluray --enable-libmp3lame --enable-libopus \
--enable-librubberband --enable-libsnappy --enable-libtesseract --enable-libtheora \
--enable-libvidstab --enable-libvorbis --enable-libvpx --enable-libwebp --enable-libx264 \
--enable-libx265 --enable-libxvid --enable-lzma --enable-libfontconfig --enable-libfreetype \
--enable-frei0r --enable-libass --enable-libopencore-amrnb --enable-libopencore-amrwb \
--enable-libopenjpeg --enable-librtmp --enable-libspeex --enable-libsoxr \
--enable-videotoolbox --disable-libjack --disable-indev=jack
```

编译之后，惊喜的发现，虽然`ffprobe`命令依旧异常，但是`ffprobe_g`命令已经可以使用了。具体如下图所示：

![](6.jpg)

`ffprobe`和`ffprobe_g`的不同就是`ffprobe_g`会包含调试信息。

回顾编译指令的最后几条指令：

```
LD ffmpeg_g
LD ffprobe_g
LD ffplay_g
STRIP ffmpeg
STRIP ffprobe
STRIP ffplay
```

可以发现，在编译的最后，使用`strip`命令，去掉了`ffprobe`中的调试信息。那么是否是`strip`命令有问题呢？

带着这个疑问，写了一段测试代码`test.cpp`：

```c++
#include <iostream>

int main() {
    std::cout << "Hello World!" << std::endl;
    return 0;
}
```

使用`clang++`编译并对比利用`strip`命令去掉调试信息前后的运行结果，终于发现了问题所在：

![](7.jpg)

至此，终于找到原因：`strip`命令生成的无调试信息的可执行程序，会发生`Malformed Mach-o file`的现象。

## 修正strip版本
怀疑机器上的`strip`版本异常导致去掉调试信息后的可执行文件出现异常。

```
$ which strip
/usr/local/opt/binutils/bin/strip
$ which clang++
/usr/bin/clang++
$ ls /usr/bin/strip
/usr/bin/strip
$ /usr/bin/strip -x a.out
$ ./a.out
Hello World!
```

经过如上的过程发现，确实是`strip`的版本有问题，利用`/usr/bin/strip`生成的无调试信息的可执行程序是正常的。

## 问题解决
在编译ffmpeg时，利用`--strip`选项来指定`strip`的版本。

```
 ./configure --enable-shared --enable-pthreads --enable-version3 --enable-avresample \
--cc=clang --host-cflags= --host-ldflags= --enable-ffplay --enable-gnutls --enable-gpl \
--enable-libaom --enable-libbluray --enable-libmp3lame --enable-libopus \
--enable-librubberband --enable-libsnappy --enable-libtesseract --enable-libtheora \
--enable-libvidstab --enable-libvorbis --enable-libvpx --enable-libwebp --enable-libx264 \
--enable-libx265 --enable-libxvid --enable-lzma --enable-libfontconfig --enable-libfreetype \
--enable-frei0r --enable-libass --enable-libopencore-amrnb --enable-libopencore-amrwb \
--enable-libopenjpeg --enable-librtmp --enable-libspeex --enable-libsoxr \
--enable-videotoolbox --disable-libjack --disable-indev=jack \
--strip=/usr/bin/strip
```

大功告成，问题解决，具体如下所示：

![](8.jpg)

