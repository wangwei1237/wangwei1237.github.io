---
title: 使用Wireshark分析SRT直播流
reward: false
top: false
date: 2020-04-09 12:33:36
categories: 
  - 视频技术
tags:
  - Wireshark
  - SRT
  - 网络协议分析
---

[SRT(Secure Reliable Transport)](https://www.haivision.com/products/srt-secure-reliable-transport/)是一种基于[UDT(UDP-based Data Transfer)](https://tools.ietf.org/html/draft-gg-udt-03)的、安全的、可靠的、开源的数据传输协议&技术。SRT在UDP基础之上实现了：智能数据重传机制和AES256加密技术，这使得其成为一种安全、可靠、低延迟的传输技术。利用SRT，可以实现在不可预测的网络环境下（例如互联网）高效、安全的传输数据。[SRT](https://github.com/Haivision/srt/)还做了特殊优化以适合视频实时流数据的传输。根据[SRT Alliance](https://www.srtalliance.org/srt-alliance-announces-the-addition-of-the-srt-low-latency-protocol-to-open-broadcaster-softwares-obs-studio/)在2019-04-04的介绍，目前如下的应用已经集成并支持SRT：[OBS Studio](https://obsproject.com/)，[VideoLAN's VLC](https://www.videolan.org/vlc/)，[FFMpeg](http://ffmpeg.org/)，[Wireshark](https://www.wireshark.org/)。

![](1.png)
<!--more-->

本文只介绍：**如何利用FFMpeg生成SRT数据流并利用Wireshark对该SRT数据进行抓包分析**。关于SRT的详细内容，可以参考[SRT Protocol Technical Overview Draft](https://github.com/Haivision/srt/files/2489142/SRT_Protocol_TechnicalOverview_DRAFT_2018-10-17.pdf)。

## 前期准备

1. 按照[说明](https://github.com/Haivision/srt/blob/master/README.md)安装SRT
2. 利用`./configure --enable-libsrt`重新编译FFMpeg，让ffmpeg工具集支持SRT协议。重新configure的过程如果遇到`ERROR: srt >= 1.3.0 not found using pkg-config`的错误，可以查看`ffbuild/config.log`的相关信息，一般需要把srt和srt所依赖的openssl的**pkgconfig**路径增加到`PKG_CONFIG_PATH`环境变量中即可。
3. 升级Wireshark到3.0之后的版本，并且设置Wireshark取消Wireshark对UDT协议的支持，具体做法为：点击菜单栏中的`Analyze`->`Enabled Protocols`，然后从弹出的支持协议中找到UDT，并取消UDT前面的选择标记。

    ![](2.jpg)

4. 安装VLC播放器，用于播放SRT协议的视频流。

## 生成SRT直播流
可以利用`ffmpeg`和`srt-live-transmit`（该工具在安装srt的时候会默认安装）来生成SRT直播流。主要思路是首先利用`ffmpeg`生成UDP的直播流，然后利用`srt-live-transmit`把UDP的直播流转换成SRT的直播流，更详细的方式可以参考[srt-live-transmit的使用说明](https://github.com/Haivision/srt/blob/master/docs/srt-live-transmit.md)。

```shell
# 生成UDP视频流
$ ffmpeg -f lavfi -re -i smptebars=duration=300:size=1280x720:rate=30 \
-f lavfi -re -i sine=frequency=1000:duration=60:sample_rate=44100 \
-pix_fmt yuv420p -c:v libx264 -b:v 1000k -g 30 -keyint_min 120 \
-profile:v baseline -preset veryfast -f mpegts \
"udp://127.0.0.1:5000?pkt_size=1316"

# 生成SRT视频流
$ srt-live-transmit -s:10 udp://:5000 srt://:5001

# 使用ffplay播放SRT视频流
$ ffplay "srt://127.0.0.1:5001"
```

具体播放效果如下所示：

![](3.jpg)

## 使用Wireshark分析SRT
为了可以用Wireshark抓到SRT数据包，需要使用VLC播放器来打开刚才创建的SRT视频流，具体如下所示：

![](4.jpg)

打开Wireshark，选择Lookback（因为要捕获的SRT地址为127.0.0.1），然后在捕获的数据窗口选择srt协议过滤，稍等片刻就可以看到捕获的SRT数据包，具体如下图所示：

![](5.jpg)

接下来就可以利用Wireshark来分析SRT协议的处理流程，例如上图中的Handshake数据包。尤其是在学习的过程中，配合[SRT的协议文档](https://github.com/Haivision/srt/files/2489142/SRT_Protocol_TechnicalOverview_DRAFT_2018-10-17.pdf)以及Wireshark的抓包分析，能够加深对SRT协议的理解，达到事半功倍的效果。

