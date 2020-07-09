---
title: 使用Boost::Python用C++开发Python库
reward: false
top: false
date: 2020-06-13 15:48:18
categories:
  - python
tags:
  - Boost::Python
  - setup.py
  - distutils
---

## 背景
很多系统都会用`json`格式进行数据交互。为了保证数据在系统上下游的自动校验，避免数据结构异常带来的系统稳定性问题，可以采用[`json-schema`](http://json-schema.org)来定义`json接口`，并利用[`json-schema-validator`](https://datatracker.ietf.org/doc/draft-handrews-json-schema-validation)来校验接口响应的结构的合法性。

然而系统中不同子系统的实现（编程语言）并非总是一致，虽然各种语言都提供了[`json-schema-validator`](https://datatracker.ietf.org/doc/draft-handrews-json-schema-validation)的具体实现，但是不同语言支持的`json-schema-validator`标准的版本并非完全一致，这会对后续的使用带来一些混乱，例如：

* Java: [json-schema-validator](https://github.com/java-json-tools/json-schema-validator)支持[Draft 4](https://tools.ietf.org/html/draft-fge-json-schema-validation-00)
* C++: [json-schema-validator](https://github.com/pboettch/json-schema-validator)支持[Draft 7](https://tools.ietf.org/html/draft-handrews-json-schema-validation-01)
* Python [jsonschema](https://github.com/Julian/jsonschema)支持[Draft 7](https://tools.ietf.org/html/draft-handrews-json-schema-validation-01)

<!--more-->

目前大多数语言，例如：Java, Golang, Php, Python, Lua等都支持C/C++的扩展。因此，可以用C/C++来为其他语言提供统一的扩展库支持，本文就是介绍怎么利用`Boost::Python`库提供[`json-schema-validator`](https://github.com/pboettch/json-schema-validator)的Python支持，并介绍如何利用`distutils`来编译&分发Python库。

![](1.png)

## Python调用C/C++的方式
有两种方式可以实现Python调用C/C++编写的库：

* 使用Python扩展
* 使用`ctypes`模块直接加载C/C++编写的so

#### ctypes加载so文件
`ctype`是Python的外部函数库。它提供了C兼容的数据类型，并允许在DLL或共享库中调用函数。

`ctype`是Python封装的API函数库。其中`cdll = <ctypes.LibraryLoader object>`是一个库加载器对象，调用`cdll.LoadLibrary`便可调用C/C++的so库。

此处不再对如何使用`ctype`模块加载so文件做过多的介绍，具体可以参考其[官方文档](https://docs.python.org/3/library/ctypes.html#module-ctypes)。

#### 使用Python扩展
该方式是Python为整合其它语言而提供的一种扩展机制，并且该机制不局限于C/C++，也可以用于其它语言。Python的可扩展性具有如下的优点：
* 方便为语言增加新功能
* 具有可定制性
* 可以实现代码复用
* ……

该方式的具体使用此处也不再做过多介绍，具体信息可以直接参考Python的官方文档[**Building C and C++ Extensions**](https://docs.python.org/3/extending/building.html)来了解。接下来要讲的例子就是利用了`可以实现代码复用`的优点。

如[**Building C and C++ Extensions**](https://docs.python.org/3/extending/building.html)所示，Python提供了`Python/C++ API`用来实现Python和C++的交互。然而，直接使用这些API来完成Python和C/C++的交互还是会存在很多重复工作的。`Boost::Python`则对`Python/C++ API`进行了抽象和包装，从而使得Python和C++的交互更加方便。

## Boost::Python封装C/C++扩展
接下来我们介绍我们是如何利用Boost::Python来为[nlohmann_json_schema_validator](https://github.com/pboettch/json-schema-validator)增加Python扩展。

`nlohmann_json_schema_validator` is a C++ library for validating JSON documents based on a **JSON Schema** which itself should validate with **draft-7 of JSON Schema Validation**.

#### 编译nlohmann json schema validator库
`nlohmann json schema validator`支持产出可执行的bin文件以及可供其他项目使用的动态库。为了可以将该扩展包装成Python扩展，我们需要生成该库的动态库。

首先根据`nlohmann json schema validator`的[编译文档](https://github.com/pboettch/json-schema-validator/blob/master/README.md#building-as-shared-library)编译出`nlohmann json schema validator`的动态库。

![](2.jpg)

#### Boost::Python封装C/C++代码
在安装Boost的时候，虽然Boost的头文件中存在Boost::Python相关的hpp文件，但是默认却没有该动态库。因此，在使用Boost::Python之前，首先需要安装该库。

```
$ brew install boost-python3
```

然后，我们用C++编写`class json_validator_python;`来封装`nlohmann json schema validator`库，并利用Boost::Python来导出，具体如下：

```c++
// jsv_python.cpp

#include <iomanip>
#include <iostream>

#include <nlohmann/json-schema.hpp>
#include <boost/python.hpp>

using namespace boost::python;

class JSON_SCHEMA_VALIDATOR_API json_validator_python
{
private:
    nlohmann::json_schema::json_validator validator;

public:
    json_validator_python() {}
    void set_root_schema(const std::string &json_string) 
    {
        validator.set_root_schema(nlohmann::json::parse(json_string.begin(), json_string.end()));
    }

    void validate(const std::string &json_string) const
    {
        validator.validate(nlohmann::json::parse(json_string.begin(), json_string.end()));
    }
};

// json_schema_validator为module名，必需和生成的so库名一样
BOOST_PYTHON_MODULE(json_schema_validator)
{
    class_<json_validator_python, boost::noncopyable> ("json_validator_python")
            .def("set_root_schema", &json_validator_python::set_root_schema)
            .def("validate", &json_validator_python::validate)
            ;
}
```

如上所示的`BOOST_PYTHON_MODULE`代码，目的是导出类及成员方法，这些导出的类和方法可以在Python中调用。

关于Boost::Python更详细的使用，可以参考其[官方文档](https://www.boost.org/doc/libs/1_66_0/libs/python/doc/html/index.html)。

#### 编译并产出Python扩展
```
g++ --std=c++11 -fPIC -c jsv_python.cpp \
-I${json-schema-validator_PATH}/src \
-I${nlohmann-json_PATH} \
-I${Boost_PATH}/include \
-I${Python_INCLUDE_PATH}

g++ --std=c++11 -shared \
-L${json-schema-validator_LIB_PATH} \
-L${Python_LIB_PATH} \
-L${Boost_Python_LIB_PATH} \
-lnlohmann_json_schema_validator \
-lboost_python38 -lpython3.8 \
-o json_schema_validator.so jsv_python.o
```

如上的指令，会生成封装之后的Python扩展：`json_schema_validator.so`。

#### 测试python扩展
在当前目录执行如下的Python代码，可以发现，我们封装的扩展已经可以当做Python扩展来导入并使用了。

```python
import json_schema_validator as jsv

validator = jsv.json_validator_python()


isValidator = True
try:
    validator.set_root_schema('{"type":"object", "properties": {"a":{"type": "string"}}}')
    validator.validate('{"a":"1"}');
except:
    isValidator = False

print(isValidator)
```

## 使用distutils编译并分发扩展
为了使用方便，使用[**Building C and C++ Extensions**](https://docs.python.org/3/extending/building.html)介绍的`distutils模块`编译Python扩展。

```python
from distutils.core import setup, Extension
import os

os.environ["CC"] = "g++"
os.environ["CXX"] = "g++"

module1 = Extension('json_schema_validator',
                    include_dirs = ['../../src',
                        '../../',
                        '/usr/local/Cellar/boost/1.72.0_3/include',
                        '/usr/local/opt/python@3.8/Frameworks/Python.framework/Versions/3.8/include/python3.8'],
                    libraries = ['boost_python38', 'python3.8',
                        'nlohmann_json_schema_validator'],
                    library_dirs = ['/usr/local/opt/python@3.8/Frameworks/Python.framework/Versions/3.8/lib',
                        '/usr/local/Cellar/boost-python3/1.72.0_1/lib',
                        '.'],
                    sources = ['jsv_python.cpp'],
                    extra_compile_args=['--std=c++11'],
                    extra_link_args=['--std=c++11'])

setup (name = 'json-schema-validator',
       version = '1.0',
       description = 'json schema validator',
       ext_modules = [module1])
```

```
$ python3 setup.py build
$ python3 setup.py install
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/python3.7/site-packages/json_schema_validator
```

然后就可以在任意位置，执行[测试python扩展](#测试python扩展)一节的测试代码啦。
