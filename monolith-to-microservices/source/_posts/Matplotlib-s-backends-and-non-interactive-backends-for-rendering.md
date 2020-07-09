---
title: matplotlib的backends以及非交互式绘图
reward: false
top: false
date: 2020-04-28 18:40:01
categories: 
  - matplotlib
tags:
  - python
  - matplotlib
  - non-interactive rendering 
---

在分析视频的psnr时，需要用到`matplotlib`来绘制视频每一帧的psnr。在Mac调试的时候可以正常生成如下所示的psnr趋势图：

![](1.png)

但是将应用部署到linux机器的时候，却提示**ModuleNotFoundError: No module named '_tkinter'**的错误。

<!--more-->

具体如下图所示：

![](2.jpg)

请教了同事后采用如下的方式解决了问题：

```
import matplotlib 
matplotlib.use('Agg')
import matplotlib.pyplot as plt
```

![](3.jpg)

虽然解决了问题，但是当时没有明白：
* 问题究竟是如何解决的？
* 为什么这样就可以解决？
* 解决方案和之前的写法的差异在哪里？

因此查阅了matplotlib的文档，然后就有了该文接下来的内容。为了保证材料的原汁原味，如下的内容均摘录自[matplotlib的开发文档](https://matplotlib.org/3.2.1/tutorials/introductory/usage.html#backends)。

## matplotlib中backend的概念
A lot of documentation on the website and in the mailing lists refers to the **"backend"** and many new users are confused by this term. matplotlib targets many different use cases and output formats. Some people use matplotlib interactively from the python shell and have plotting windows pop up when they type commands. Some people run Jupyter notebooks and draw inline plots for quick data analysis. Others embed matplotlib into graphical user interfaces like wxpython or pygtk to build rich applications. Some people use matplotlib in batch scripts to generate postscript images from numerical simulations, and still others run web application servers to dynamically serve up graphs.

**To support all of these use cases, matplotlib can target different outputs, and each of these capabilities is called a backend; the "frontend" is the user facing code, i.e., the plotting code, whereas the "backend" does all the hard work behind-the-scenes to make the figure.** 

There are two types of backends: 
* user interface backends (for use in pygtk, wxpython, tkinter, qt4, or macosx; also referred to as `"interactive backends"`). The interactive backends: GTK3Agg, GTK3Cairo, MacOSX, nbAgg, Qt4Agg, Qt4Cairo, Qt5Agg, Qt5Cairo, TkAgg, TkCairo, WebAgg, WX, WXAgg, WXCairo.
* hardcopy backends to make image files (PNG, SVG, PDF, PS; also referred to as `"non-interactive backends"`). The non-interactive backends: agg, cairo, pdf, pgf, ps, svg, template.


## matplotlib中backend的默认配置
By default, **Matplotlib should automatically select a default backend which allows both interactive work and plotting from scripts**, with output to the screen and/or to a file, so at least initially you will not need to worry about the backend. **The most common exception is if your Python distribution comes without tkinter and you have no other GUI toolkit installed; this happens on certain Linux distributions**, where you need to install a Linux package named python-tk (or similar).

If, however, you want to write graphical user interfaces, or a web application server, or need a better understanding of what is going on, read on. To make things a little more customizable for graphical user interfaces, matplotlib separates the concept of the renderer (the thing that actually does the drawing) from the canvas (the place where the drawing goes). The canonical renderer for user interfaces is Agg which uses the Anti-Grain Geometry C++ library to make a raster (pixel) image of the figure; it is used by the Qt5Agg, Qt4Agg, GTK3Agg, wxAgg, TkAgg, and macosx backends. An alternative renderer is based on the Cairo library, used by Qt5Cairo, Qt4Cairo, etc.

## interactive模式的概念
Use of an interactive backend (see What is a backend?) permits--but does not by itself require or ensure--plotting to the screen. Whether and when plotting to the screen occurs, and whether a script or shell session continues after a plot is drawn on the screen, depends on the functions and methods that are called, and on a state variable that determines whether matplotlib is in "interactive mode". The default Boolean value is set by the matplotlibrc file, and may be customized like any other configuration parameter (see Customizing Matplotlib with style sheets and rcParams). It may also be set via matplotlib.interactive(), and its value may be queried via matplotlib.is_interactive(). Turning interactive mode on and off in the middle of a stream of plotting commands, whether in a script or in a shell, is rarely needed and potentially confusing, so in the following we will assume all plotting is done with interactive mode either on or off.

Interactive mode may also be turned on via matplotlib.pyplot.ion(), and turned off via matplotlib.pyplot.ioff().

**In interactive mode, pyplot functions automatically draw to the screen.** When plotting interactively, if using object method calls in addition to pyplot functions, then call draw() whenever you want to refresh the plot.

Use non-interactive mode in scripts in which you want to generate one or more figures and display them before ending or generating a new set of figures. In that case, use show() to display the figure(s) and to block execution until you have manually destroyed them.

## 如何选择backend
There are three ways to configure your backend:
* The `rcParams["backend"]` (default: 'agg') parameter in your matplotlibrc file
* The `MPLBACKEND` environment variable
* The function `matplotlib.use()`

A more detailed description is given below.

If multiple of these are configurations are present, the last one from the list takes precedence; e.g. calling `matplotlib.use()` will override the setting in your matplotlibrc.

If no backend is explicitly set, Matplotlib automatically detects a usable backend based on what is available on your system and on whether a GUI event loop is already running. On Linux, if the environment variable `DISPLAY` is unset, the "event loop" is identified as "headless", which causes a fallback to a noninteractive backend (agg).

Here is a detailed description of the configuration methods:
1. Setting `rcParams["backend"]` (default: 'agg') in your matplotlibrc file:
  ```
  backend : qt5agg   # use pyqt5 with antigrain (agg) rendering
  ```
  See also Customizing Matplotlib with style sheets and rcParams.

2. Setting the `MPLBACKEND` environment variable:

  You can set the environment variable either for your current shell or for a single script.

  On Unix:

  ```
  > export MPLBACKEND=qt5agg
  > python simple_plot.py

  > MPLBACKEND=qt5agg python simple_plot.py
  ```

  On Windows, only the former is possible:

  ```
  > set MPLBACKEND=qt5agg
  > python simple_plot.py
  ```

  Setting this environment variable will override the backend parameter in any matplotlibrc, even if there is a matplotlibrc in your current working directory. Therefore, setting MPLBACKEND globally, e.g. in your .bashrc or .profile, is discouraged as it might lead to counter-intuitive behavior.

3. If your script depends on a specific backend you can use the function `matplotlib.use()`:

  ```
  import matplotlib
  matplotlib.use('qt5agg')
  ```
  This should be done before any figure is created; otherwise Matplotlib may fail to switch the backend and raise an ImportError.

  Using use will require changes in your code if users want to use a different backend. Therefore, you should avoid explicitly calling use unless absolutely necessary.

## matplotlib.use('Agg')的作用
在介绍完整整体的matplotlib中的backends以及matplotlib中的绘图模式之后，就能明白为什么文章开始出现的迁移过程中会出现类似的错误。

首先根据[matplotlib中backend的默认配置](#matplotlib中backend的默认配置)中的介绍可以知道，matplotlib会有一个默认的backend配置，并且该默认的backend会同时支持交互式绘图模式以及利用脚本绘图的模式。并且，交互式绘图还需要利用GUI工具来生成一个屏幕绘图的窗口。而迁移的Linux机器上缺乏tkinter这样的GUI工具，因此在`import matplotlib.pyplot as plt`的时候会产生错误。

而利用`matplotlib.use('Agg')`可以切换matplotlib的backend，将其backend从默认的交互式模式切换为非交互式模式，此时，生成的图形会以图片的形式保存起来，而无需tkinter这样的GUI工具的支持，因此可以解决本文最开始出现的问题。

