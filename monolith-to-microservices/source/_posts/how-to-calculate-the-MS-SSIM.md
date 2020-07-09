---
title: 如何计算MS-SSIM
reward: false
date: 2020-02-25 15:08:40
categories: 视频技术
tags:
  - SSIM
  - MS-SSIM
---

## SSIM的本质及其缺点
在[FFMpeg如何计算图像的SSIM](/2020/02/15/how-to-calculate-the-SSIM-in-FFMpeg/)中，详细介绍了$SSIM$的相关概念，并对FFMpeg中的$SSIM$实现做了详细的分析。$SSIM$算法基于HVS更擅长从图像中提取结构信息的事实，并且利用结构相似度来计算图像的感知质量。在Z. Wang等人的论文**Multi-scale structural similarity for image quality assessment**中也提到，$SSIM$算法要好于当时的其它的感知图像质量指标。

就其本质而言，$SSIM$是一种单尺度的算法，但是实际上正确的图像尺度取决于用户的观看条件，例如显示设备的分辨率，用户的观看距离等。因此，用单尺度的$SSIM$算法来评估图像的感知质量也存在其缺点。

<!--more-->

## MS-SSIM的基本概念
图像细节的可感知性取决于：
* 图像信号的采样密度
* 用户的观看距离
* HVS的感知能力

当如上的因素发生变化时，则对给定图像的主观评估也会随之发生变化。单尺度的SSIM算法可能仅适用于某个特定的配置。为了解决该问题，论文**Multi-scale structural similarity for image quality assessment**在$SSIM$算法的基础上提出了如图1所示的多尺度的结构相似性评估算法，即$MS-SSIM$算法。

![图1. MS-SSIM算法](1.jpg)

图1. MS-SSIM算法，L 表示低通滤波器，2↓ 表示采样间隔为2的下采样

因此，$MS-SSIM$实际上是一种以不同分辨率合并图像细节的图像质量评估方法。对于$MS-SSIM$，原始图像的$scale=1$，图像的最大$scale=M$。对$scale=j$的尺度而言，其亮度、对比度、结构的相似性分别表示为：
* $l_j(X,Y)$
* $c_j(X,Y)$
* $s_j(X,Y)$

因此，根据图1可以得到$MS-SSIM$的计算方式。

$$
MS-SSIM(X,Y)=\big[l_M(X,Y)\big]^{\alpha_M} \cdot \prod _{j=1}^{M}  {\big[c_j(X,Y)\big]^{\beta_j}\big[s_j(X,Y)\big]^{\gamma_j}}
$$

一般，令$\alpha_j=\beta_j=\gamma_j \ \ , \ j \in [1, M]$，我们得到：

$$
MS-SSIM(X,Y)=\big[l_M(X,Y)\big]^{\alpha_M} \cdot \prod _{j=1}^{M}  {\big[c_j(X,Y) \cdot s_j(X,Y)\big]^{\alpha_j}}
$$

**Multi-scale structural similarity for image quality assessment**给出了一种计算各尺度参数的方法，并同时给出不同尺度的参数值：
* $\alpha_1=0.0448$
* $\alpha_2=0.2856$
* $\alpha_3=0.3001$
* $\alpha_4=0.2363$
* $\alpha_5=0.1333$

## MS-SSIM算法的实现
采用FFMpeg中的$SSIM$的实现方式来实现$MS-SSIM$。根据[FFMpeg如何计算图像的SSIM](/2020/02/15/how-to-calculate-the-SSIM-in-FFMpeg/)的介绍：

$$
MS-SSIM(X,Y)=\bigg(\frac{2s1 \cdot s2 + ssimC_{1}}{s1^2+s2^2+ssimC_{1}}\bigg)^{\alpha_{M}} \cdot \prod _{j=1}^{M}\bigg(\frac{2covar + ssimC_2}{vars + ssimC_2}\bigg)^{\alpha_j}
$$

对于图像采样而言，采用简单的$2 \times 2$的卷积核执行图像的下采样，：

$$
\begin{bmatrix}
\frac{1}{4} & \frac{1}{4} \\\\
\frac{1}{4} & \frac{1}{4} \\\\
\end{bmatrix}
$$

具体的采样代码如下所示：

```
void downsample_2x2_mean(pixel *input, int width, int height, pixel *output) {
    int downsample_width =  width >> 1;
    int downsample_height = height >> 1;

    for (int y = 0; y < downsample_height; y++) {
        for (int x =0; x < downsample_width; x++) {
            output[y * downsample_width + x] = (input[2 * y * width + 2 * x] +
                                                input[2 * y * width + 2 * x + 1] +
                                                input[(2 * y + 1) * width + 2 * x] +
                                                input[(2 * y + 1) * width + 2 * x + 1]) / 4;
        }
    }
}
```

然后利用[tiny_ssim](https://github.com/FFmpeg/FFmpeg/blob/master/tests/tiny_ssim.c)中的`ssim_plane()`迭代计算$MS-SSIM$。

```
float ms_ssim_plane(pixel *pix1, pixel *pix2, int width, int height, int scale) {
    for (int i = 0; i < w * h; i++) {
        ori_img1[i] = pix1[i];
        ori_img2[i] = pix2[i];
    }

    // 计算每个尺度的ssim值.
    for (int i = 1; i <= scale; i++) {
        if (i != 1) {
            downsample_2x2_mean(ori_img1, w, h, sample_img1);
            downsample_2x2_mean(ori_img2, w, h, sample_img2);
            w = w >> 1;
            h = h >> 1;
            for (int j = 0; j < w * h; j++) {
                ori_img1[j] = sample_img1[j];
                ori_img2[j] = sample_img2[j];
            }
        }
        value = ssim_plane(ori_img1, w, ori_img2, w, w, h, temp, NULL);
        result *= pow(value.C_S, WEIGHT[i-1]);
        luminance_value[i-1] = value.L;
    }

    result *= pow(luminance_value[scale-1], WEIGHT[scale-1]);
    return result;
}
```

完整的代码可以参考[test_msssim.cpp](https://gitee.com/wangwei1237/wangwei1237/blob/master/2020/02/25/how-to-calculate-the-MS-SSIM/test_msssim.cpp)。代码的很大部分是由我的同事*贤杰*(github: [@bodhisatan](https://github.com/bodhisatan))实现的，在此一并表示感谢。
