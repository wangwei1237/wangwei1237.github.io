---
title: FFMpeg如何计算图像的SSIM
reward: false
date: 2020-02-15 15:08:41
categories: 视频技术
tags:
  - SSIM
  - FFMpeg
---

## SSIM基本概念
关于$SSIM$的具体解释，此处不再介绍，具体可以参见[数字视频相关概念](https://wangwei1237.gitee.io/digital-video-concept/)中的[SSIM算法](https://wangwei1237.gitee.io/digital-video-concept/docs/4_2_2_3_StructuralSimilarityBasedApproaches.html)一节的介绍。

直接给出$SSIM$的计算方法：
$$
SSIM(x,y)=\frac{(2\mu_x\mu_y+C_1)(2\sigma_{xy}+C_2)}{(\mu_x^2+\mu_y^2+C_1)(\sigma_x^2+\sigma_y^2+C_2)}
$$

$C_1=(K_1L)^2, C_2=(K_2L)^2$。$K_1\ll1$，$K_2\ll1$均为常数，计算时，一般$K_1=0.01$，$K_2=0.03$。$L$是灰度的动态范围，由图像的数据类型决定，如果数据为*uint8*，则$L=255$。

<!--more-->

## SSIM计算中的图像分割
在整幅图片的跨度上，图像亮度的均值和方差变化较为剧烈；并且图像上不同区块的失真程度也有可能不同；再者人眼睛每次只能聚焦于一处，更关注局部数据而非全局数据。因此如上的$SSIM$算法不能直接作用于一整副图像。

在论文**Image quality assessment: From error visibility to structural similarity**中，作者采用$11 \times 11$的滑动窗口将整副图像分割为$N$个patch，然后计算每一个patch的$SSIM$，最后计算所有patch的$SSIM$值的平均数（$Mean \ \ SSIM:MSSIM$）作为整副图像的$SSIM$。为了避免滑动窗口带来的块效应，在计算每个patch的均值$\mu$和方差$\sigma^2$时，作者采用了$\sigma=1.5$的高斯卷积核作加权平均。

如果整副图像有$N$个patch，则$MSSIM$的计算方式为：

$$
MSSIM(X,Y) = \frac{1}{N}\sum_{i=1}^{N}{SSIM(x_i, y_i)}
$$

其中，$SSIM(x_i, y_i)$为第$i$个patch的$SSIM$。

## FFMpeg中计算SSIM的算法
在FFMpeg中，也提供了计算$SSIM$的实现：[tiny_ssim](https://github.com/FFmpeg/FFmpeg/blob/master/tests/tiny_ssim.c)。从代码的注释中可以看到：为了提升算法的性能，没有采用论文中的高斯加权方式计算每个patch的$SSIM$，而是采用了一个$8 \times 8$的块来计算每个patch的$SSIM$。

```
/*
 * tiny_ssim.c
 * Computes the Structural Similarity Metric between two rawYV12 video files.
 * original algorithm:
 * Z. Wang, A. C. Bovik, H. R. Sheikh and E. P. Simoncelli,
 *   "Image quality assessment: From error visibility to structural similarity,"
 *   IEEE Transactions on Image Processing, vol. 13, no. 4, pp. 600-612, Apr. 2004.
 *
 * To improve speed, this implementation uses the standard approximation of
 * overlapped 8x8 block sums, rather than the original gaussian weights.
 */
```

### standard approximation of overlapped 8x8 block sums
接下来就解释一下注释中的*standard approximation of overlapped 8x8 block sums*究竟是什么含义。在解释的过程中会分解成两个部分来解释：*overlapped 8x8 block*和*sums*。

### overlapped 8x8 block的含义
FFMpeg在计算图像$SSIM$时，首先以$4 \times 4$的块大小把**图1**所示的分辨率为$W \times H$的图像：
![图1](1.jpg)
图1：原始图像

分割为**图2**的样式。
![图2](2.jpg)

图2：分割后的图像

对于**图2**中的每一块用$block(i,j)$来表示（*图2中的红色块*），FFMpeg使用$block(i,j)$及其**上、右、右上块**（*图2中的绿色块*）来计算其$SSIM:SSIM(x_{ij},y_{ij})$。

$block(i,j)$及其**上、右、右上块**构成一个$8\times8$的像素块，并且该$8\times8$的块和计算$block(i,j+1)$的$SSIM$用到的$8\times8$的块存在重合像素，这就是注释中的**overlapped 8x8 block**的真正含义。

因此，根据如上规则：$i \in [1,\frac{H}{4}],j \in [0,\frac{W}{4}-1]$。也就是说：第0行和最后一列的块不会计算$SSIM$。

最后得到FFMpeg中的$SSIM$计算方式为：

$$
SSIM = MSSIM(X,Y) = \frac{1}{N}\sum_{i=1}^{\frac{H}{4}}\sum_{j=0}^{\frac{W}{4}-1}{SSIM(x_{ij}, y_{ij})}
$$

$$
N=(\frac{H}{4}-1)(\frac{W}{4}-1)
$$

### sums的含义
如前所述，我们分析了FFMpeg计算图像的$SSIM$的整体思路，接下来我们继续分析FFMpeg是如何计算$block(i,j)$的$SSIM(x_{ij},y_{ij})$的。

首先利用函数`ssim_4x4x2_core()`来计算$block(i,j)$块的结构相似性指标，主要是如下的4个指标：
* *s1*：参考图像在$block(i,j)$块的像素之和
* *s2*：受损图像在$block(i,j)$块的像素之和
* *ss*：参考图像和受损图像在$block(i,j)$块的像素平方之和
* *s12*：参考图像和受损图像在$block(i,j)$块的对应像素乘积之和

$s1=\sum_{i=0}^{3}\sum_{j=0}^{3}{x(i,j)}$
$s2=\sum_{i=0}^{3}\sum_{j=0}^{3}{y(i,j)}$
$ss=\sum_{i=0}^{3}\sum_{j=0}^{3}{\Big(\big(x(i,j)\big)^2+\big(y(i,j)\big)^2\Big)}$
$s12=\sum_{i=0}^{3}\sum_{j=0}^{3}{\big(x(i,j) \cdot y(i,j)\big)}$

如上的4个指标就是我们后续会用到的sums，该sums也就是*overlapped 8x8 block sums*中的*sums*的概念。

### 利用sums计算各块的SSIM
接下来利用该sums值计算$SSIM$。

为了提升效率，FFMpeg会按照行来计算每一行的各个块的sums数据，并将每个行块的sums数据存储在长度为$\frac{W}{4}$的数组指针sum（`(int(*)[4])`）中。

其中sum指针有两种：
* sum0：存储当前行的各块的sums结果
* sum1：存储当前行的上一行的sums结果

先计算第$i-1$行块和第$i$行块的sums结果，并分别存入`sum1`和`sum0`中。然后遍历第$i$行块的每一个块，并利用`sum1`和`sum0`中计算的结果来计算每一块的$SSIM$。

函数`ssim_end4()`展示了如何利用$block(i-1,j)$，$block(i-1,j+1)$，$block(i,j)$，$block(i,j+1)$的sums信息来计算$SSIM(x_{ij},y_{ij})$：
* 先对4个块的sums结果进行加和处理，得到$8\times8$块的sums结果
* 然后利用该$8\times8$块的sums来计算$block(i,j)$的$SSIM$

函数`ssim_end1()`就展示了如何利用$8\times8$块的sums信息来计算$SSIM$。具体的计算方法如下。

将红色区块$block(i,j)$的图像放大一点，如图3所示。我们接下来计算其$SSIM$。
![图3](3.jpg)
图3：*block(i,j)*的示意图

在计算时，首先将4个区块的sums值求和，得到$8\times8$区块的sums值，分别为：
* $s1=\sum_{i=0}^{63}{x_i}$
* $s2=\sum_{i=0}^{63}{y_i}$
* $ss=\sum_{i=0}^{63}{(x_i^2+y_i^2)}$
* $s12=\sum_{i=0}^{63}{(x_i \cdot y_i)}$

根据如上的计算，可以得到$\mu_x$，$\mu_y$，$\mu_x\mu_y$，$\mu_x^2+\mu_y^2$，$\sigma_x^2+\sigma_y^2$，$\sigma_{xy}$：
* $\mu_x=\frac{1}{64} \cdot s1$
* $\mu_y=\frac{1}{64} \cdot s2$
* $\mu_x\mu_y=\frac{1}{64 \cdot 64}(s1 \cdot s2)$
* $\mu_x^2+\mu_y^2=\frac{1}{64 \cdot 64}\big((s1)^2+(s2)^2\big)$
* $\sigma_x^2+\sigma_y^2=\frac{1}{64 \cdot 63}\big(64 \cdot ss-(s1)^2- (s2)^2\big)$
* $\sigma_{xy}=\frac{1}{64 \cdot 63}(64 \cdot s12 - s1 \cdot s2)$

利用如上的公式（具体推导可以参考[ssim_end1()的推导](/2020/02/18/the-proof-of-the-SSIM-in-FFMpeg/)）对$SSIM$的公式进行计算可以得到：
$$
SSIM(x,y)=\frac{(2\mu_x\mu_y+C_1)(2\sigma_{xy}+C_2)}{(\mu_x^2+\mu_y^2+C_1)(\sigma_x^2+\sigma_y^2+C_2)}
$$

$$
=\frac{(2s1s2+64^2C_1)(2\cdot64s12-2s1s2+64\cdot63C_2)}{(s1^2+s2^2+64^2C_1)(64ss-s1^2-s2^2+64\cdot63C_2)}
$$

FFMpeg中，对$C_1$和$C_2$的定义中的因子**64**或**63**也是根据上面的公式，但是从公式看，[**FFMpeg对`ssim_c1`的计算少乘了64**](https://trac.ffmpeg.org/ticket/8529)：
```
ssim_c1 = 0.01 * 0.01 * 255 * 255 * 64 + 0.5
ssim_c2 = 0.03 * 0.03 * 255 * 255 * 64 * 63 + 0.5
```

当然，为了简化处理，FFMpeg还做了如下的定义：
```
vars  = ss  * 64 - s1 * s1 - s2 * s2;
covar = s12 * 64 - s1 * s2;
```

因此，最终在FFMpeg中，计算$SSIM$的公式为：

$$
SSIM(x,y)=\frac{(2s1s2+ssimC_1)(2covar+ssimC_2)}{(s1^2+s2^2+ssimC_1)(vars + ssimC_2)}
$$

如上的公式就是函数`ssim_end1()`中最终的计算方式。

### 利用各块的SSIM计算图像的SSIM
计算完所有块的$SSIM$之后，就可以计算所有块的平均$SSIM$并作为该图像的$SSIM$：

$$
SSIM(X,Y)= \frac{1}{N}\sum_{i=1}^{\frac{H}{4}}\sum_{j=0}^{\frac{W}{4}-1}{SSIM(x_{ij}, y_{ij})}
$$

$$
N=(\frac{H}{4}-1)(\frac{W}{4}-1)
$$

### 编码过程中的技巧
在FFMpeg计算$SSIM$的算法实现中，为了提升效率和抽象代码逻辑，也利用了很多的编程技巧，例如：
* 计算YUV各分量图像宽度时用`w >> !!i`
* 为了避免对第0行的特殊处理，采用两层循环来处理
* 计算每一行的各块的sums信息时，为了降低循环次数，每次循环计算2个块的sums结果，`ssim_4x4x2_core`的函数名可能就是这么来的。
* 计算每一行的各块的$SSIM$时，为了降低循环次数，每次循环计算4个块的$SSIM$，`ssim_end4`的函数名可能就是这么来的。

## 感谢
分析过程离不开和贤杰(github: [@bodhisatan](https://github.com/bodhisatan))的不断讨论和交流，感谢@贤杰在繁忙的工作之余抽出时间来一起分析FFMpeg中SSIM算法的实现原理。