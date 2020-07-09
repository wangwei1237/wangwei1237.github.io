---
title: 集群时间戳不一致导致的问题及其分析和思考
reward: false
top: false
date: 2020-05-29 09:23:18
categories:
  - 总结
tags:
  - ntp
  - 时间戳不同步
---

> 1883年11月18日，美国第一个全国统一铁路时刻表诞生，这一天的正午时分，美国东部的时钟全部回拨。从此，上帝的时间被改用人间的指针来度量。 ——**公司的力量**

## 背景
最近遇到的业务上的问题涉及：*在一个集群中，存在部分机器的`时间戳`和其它机器不一致而导致的重大问题*。为了不泄露业务的相关信息，文中会用一个类似的场景来对这个问题进行分析。
<!--more-->

## 时间戳（unix time stamp）
需要注意的是，这里涉及到的问题是集群`时间戳`不一致而不是`时间`不一致。

[The unix time stamp](https://www.unixtimestamp.com) is a way to track time as a running total of seconds. This count starts at the Unix Epoch on January 1st, 1970 at UTC. Therefore, the unix time stamp is merely the number of seconds between a particular date and the Unix Epoch. It should also be pointed out (thanks to the comments from visitors to this site) that **`this point in time technically does not change no matter where you are located on the globe`**. This is very useful to computer systems for tracking and sorting dated information in dynamic and distributed applications both online and client side.

由此可知，`时间戳`和`时间`不同，是一个和时区无关的全球一致的时间计量方式。不同集群会因为其所在的时区不同而存在时间不同的问题，但是其时间戳是一致的。

Linux系统而言，时间戳可以利用[`GNU shell utilities`](https://www.gnu.org/software/shellutils/shellutils.html)提供的[`date`命令](https://github.com/coreutils/coreutils/blob/master/src/date.c)来获取。

```
$ date +%s
$ 1590719825
```

## 时间戳不一致带来的问题
考虑如下的一个购物站点的活动场景，如下的场景可能不会在现实的电商活动中存在，该场景仅仅是为了说明本文的问题而抽象的一个场景。

> 原价100￥的商品，现价10￥元销售，但前提是下单的用户需要在1分钟内完成支付。如果下单用户未能在1分钟内完成支付，则该订单无效，同时该用户无法继续以折扣价下单。

为了方便问题说明，下单和支付等功能合并在`trade`，产生的相关数据则存储于`DB`。简化之后的架构如下所示：

![](1.png)

当`trade`为单机部署时，不会存在问题。但是当`trade`作为集群部署时，则会存在集群时间戳不一致导致用户支付失败的问题。具体如下所示：

![](2.png)

如图所示，`m1`和`m2`的时间戳是一致的，但是`m3`的时间戳比其它机器正好快了**60s**。因此，如果`支付`的流量由`m3`机器的`trade`处理，则即便是在1分钟内支付，因为`m3`机器的时间戳快60s而导致支付超时，进而导致用户支付失败，影响用户体验。

这就是我们要讲的：在一个集群中，部分机器的`时间戳`和其它机器不一致而导致的重大问题。

## 问题如何解决
在不修改系统架构的前提下，则可以利用同一时钟同步集群中所有机器的时钟，从而让集群的时钟保持同步，进而解决该问题。具体的方案如下图所示。

![](3.png)

### ntp
对Linux而言，可以使用[`ntp`](https://en.wikipedia.org/wiki/Network_Time_Protocol)来实现集群的时间同步。

同步时间，可以使用`ntpdate`命令，也可以使用`ntpd`服务。

### ntpdate
使用`ntpdate`比较简单。格式如下：

```
$ ntpdate [-nv] [NTP IP/hostname]
$ ntpdate 192.168.0.1
$ ntpdate time.ntp.org
```

`ntpdate`只是强制将系统时间设置为ntp服务器时间。如果机器cpu tick有问题，这种同步治标不治本。此时，一般配合`cron`任务来定期同步。例如：

```
0 12 * * * * /usr/sbin/ntpdate 192.168.0.1
```

### ntpd
使用`ntpd`服务，要好于使用`ntpdate+cron`。

`ntpdate`同步时间，会造成时间的跳跃，对一些依赖时间的程序和服务会造成影响，比如sleep，timer等。

`ntpd`服务可以在修正时间的同时，修正cpu tick。

理想的做法为，在开机的时候，使用`ntpdate`强制同步时间，在其他时候使用`ntpd`服务来同步时间。

需要注意的是，`ntpd`有一个自我保护设置: 如果本机时间与源时间相差太大, 则`ntpd`不运行。所以新设置的时间服务器一定要先利用`ntpdate`命令获取初始时间, 然后启动`ntpd`服务。

关于ntpd的使用，网上已经有很多资料了，这里就不做过多的介绍了。

### ntpstat
`ntpstat`用于显示网络时间的同步状态。`ntpstat`会给出本地计算机上运行的`ntpd`的同步状态。如果本地系统与参考时间源同步，则`ntpstat`会给出近似的时间精度。

```
$ ntpstat
synchronised to NTP server (192.168.0.1) at stratum 3
   time correct to within 43 ms
   polling server every 1024 s
```

## 问题引申
虽然可以利用`ntp`来同步集群时间，但是有一个问题是值的思考的：**如何在集群时钟不同的情况下，解决[时间戳不一致带来的问题](#时间戳不一致带来的问题)一节中描述的问题**。

系统在架构上要做何种优化，才能避免业务对集群时钟同步的依赖？而这种优化所带来的系统架构的复杂度和成本又将如何？
