---
title: 使用hexo和github搭建个人博客
reward: false
date: 2020-02-05 20:01:29
categories: 技术
tags: 
  - hexo
  - github
  - 建站
---

## 创建github仓库
首先打开[github](https://github.com/)，点击`New repository`，创建一个新仓库用于存储博客的所有内容。

仓库名必须为：`账户名.github.io`，并且需要勾选**Initialize this repository with a README**。

在建好的仓库右侧有个**settings**按钮，点击**settings**，向下滑动到**GitHub Pages**，会发现有个网址，github会把该仓库中的项目部署到该网址下，该网址也是博客的默认地址。当然也可以购买域名，将其换成喜欢的地址。

<!--more-->

![](1.jpg)

![](2.jpg)



## 准备node.js环境
由于hexo是基于[node.js](https://nodejs.org/en/)开发的，因此在安装hexo之前，需要先安装node.js并配置响应的node.js环境。具体如下所示：

![](3.jpg)

## 准备hexo环境
根据[hexo官网](https://hexo.io/)的提示，安装hexo。具体如下所示：

```
$ npm install hexo-cli -g
$ hexo init blog
$ cd blog
$ npm install
$ hexo server
```
此时hexo会在4000端口启动一个webserver，用浏览器访问`localhost:4000`则会看到初始化之后默认的站点。

## 安装hexo插件
### hexo-generator-category
该插件用于自动生成category，具体安装方式如下：

```
$ npm install hexo-generator-category --save
$ hexo new page categories
```
然后修改`source/categories/index.md`的内容为：

```
---
title: categories
date: 2020-02-05 11:53:35
type: "categories"
layout: "categories"
comments: false
---
```

### hexo-generator-tag
该插件用于自动生成tags，具体安装方式如下：

```
$ npm install hexo-generator-tag --save
$ hexo new page tags
```
然后修改`source/tags/index.md`的内容为：

```
---
title: tags
date: 2020-02-05 11:53:58
type: "tags"
layout: "tags"
comments: false
---
```

### hexo-asset-image
该插件主要用于解决在文章中引用图片的场景，具体安装方式如下：

```
$ npm install hexo-asset-image --save
```

安装该插件后，利用`hexo new post 'test'`生成新文章内容时，会在`source/_posts`目录下同时生成`test.md`和`test目录`，`test.md`中需要的图片可以存储在`test目录`下。在文档中，采用如下的方式来引用图片：

```
![图片的描述信息](1.jpg)
```

关于hexo-asset-image生成图片的bug，可以参考文章：[解决hexo-asset-image的图片地址错误问题](/2020/02/05/handle-the-bug-of-hexo-asset-image-plugin/)

### hexo-deployer-git
该插件用于将编译后生成的站点内容发布到第一步创建的github仓库中，从而实现内容更新。具体安装方式如下：

```
$ npm add hexo-deployer-git
```

然后配置_config.yml中的`url`和`deploy`配置，以便可以通过该插件自动部署项目。具体配置如下所示：

```yaml
url: https://wangwei1237.github.io/
deploy:
  type: git
  repo: https://github.com/wangwei1237/wangwei1237.github.io
```

配置完毕之后，如果项目内容发生修改，则利用如下的命令就可以完成自动发布：

```
$ hexo clean
$ hexo generate
$ hexo deploy
```