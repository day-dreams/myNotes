如何使用cmake构建项目
<!-- TOC -->

- [1. 常用语法](#1-常用语法)
    - [1.1. 项目设定类](#11-项目设定类)
        - [1.1.1. CMAKE_MINIMUM_REQUIRED](#111-cmake_minimum_required)
        - [1.1.2. PROJECT()](#112-project)
        - [1.1.3. SET()](#113-set)
        - [1.1.4. ADD_EXECUTABLE()](#114-add_executable)
        - [1.1.5. ADD_SUBDIRECTORY()](#115-add_subdirectory)
        - [1.1.6. INCLUDE_DIRECTORIES()](#116-include_directories)
        - [1.1.7. LINK_DIRECOTRIES()](#117-link_direcotries)
        - [1.1.8. TARGET_LINK_LIBRARIES()](#118-target_link_libraries)
        - [1.1.9. AUX_SOURCE_DIRECTORY()](#119-aux_source_directory)
    - [1.2. 信息输出类](#12-信息输出类)
        - [1.2.1. MESSAGE()](#121-message)
- [2. 常用命令](#2-常用命令)
    - [2.1. 清理构建:make clean](#21-清理构建make-clean)
    - [2.2. 外部构建:out-of-souce](#22-外部构建out-of-souce)
- [3. 常用变量](#3-常用变量)
    - [3.1. EXECUTABLE_OUTPUT_PATH](#31-executable_output_path)
    - [3.2. LIBRARY_OUTPUT_PATH](#32-library_output_path)
    - [3.3. CMAKE_C_FLAGS](#33-cmake_c_flags)
    - [3.4. CMAKE_CXX_FLAGS](#34-cmake_cxx_flags)
    - [3.5. CMAKE_SOURCE_DIR](#35-cmake_source_dir)
- [4. 一个例子](#4-一个例子)

<!-- /TOC -->

# 1. 常用语法

## 1.1. 项目设定类
### 1.1.1. CMAKE_MINIMUM_REQUIRED
设定cmake最低版本
```cmake
CMAKE_MINIMUM_REQURED(VERSION 3.0 FATAL_ERROR)#版本低于3.0则直接抛出致命错误，构建停止
```
### 1.1.2. PROJECT()
```cmake
PROJECT(projectname [CXX][C][Java])
PROJECT(HELLO)
```
这条命令用来设定项目名字，会同时建立两个变量:projectname_BINARY_DIR,projectname_SOURCE_DIR
### 1.1.3. SET()
这条命令用于定义一些变量
```cmake
SET(VAR [VALUE] [ CAHCE TYPE DOCSTRING [FORCE]])
SET(SRC_LIST main.c t1.c t2.c) 
```
### 1.1.4. ADD_EXECUTABLE()
用于指定生成的可执行文件
```cmake
ADD_EXECUTABLE(demo ${SRC_LIST})
ADD_EXECUTABLE(demo main.c)

```
### 1.1.5. ADD_SUBDIRECTORY()
用于添加子目录,EXCLUDE_FROM_ALL参数用于排除目录
```cmake
ADD_SUBDIRECTORY(source_dir [binary_dir] [EXCLUDE_FROM_ALL])
ADD_SUBDIRECTORY(my_src my_bin)#指定源代码目录为my_src,可执行文件输出目录为my_bin
```
### 1.1.6. INCLUDE_DIRECTORIES()
用于向工程添加多个特定的头文件搜索目录。AFTER和BEFORE用来指定是最先还是最后搜索本目录
```cmake
INCLUDE_DIRECTORIES([AFTER|BEFORE] [SYSTEM] dir1 dir2 ...)
INCLUDE_DIRECTORIES(myworkplace/project/include/)
```
### 1.1.7. LINK_DIRECOTRIES()
向工程添加共享库的搜索路径。
### 1.1.8. TARGET_LINK_LIBRARIES()
向目标添加需要链接的共享库。
```cmake
TARGET_LINK_LIBRARIES(target library1 <debug | optimized> library2 ...)

TARGET_LINK_LIBRARIES(demo hello)#共享库/静态库的名字是hello

TARGET_LINK_LIBRARIES(demo libhello.so)#共享库的文件名是libhello.so
TARGET_LINK_LIBRARIES(demo libhello.a)#静态库的文件名是libhello.a
```
### 1.1.9. AUX_SOURCE_DIRECTORY()
查找某路径下的所有源文件，存储在制定变量中。可以避免手工罗列所有文件
```cmake
AUX_SOURCE_DIRECTORY(<dir> <variable>)
AUX_SOURCE_DIRECTORY(mysrc/ src_list)
```
## 1.2. 信息输出类

### 1.2.1. MESSAGE()

```cmake
MESSAGE(STATUS "用来输出一些信息")
MESSAGE(SEND_ERROR "发生了某种错误，将跳过本步构建")
MESSAGE(FATAL_ERROR "发生了致命错误，构建将被终止")
```

# 2. 常用命令

## 2.1. 清理构建:make clean
## 2.2. 外部构建:out-of-souce

# 3. 常用变量
## 3.1. EXECUTABLE_OUTPUT_PATH
生成的可执行文件输出路径
## 3.2. LIBRARY_OUTPUT_PATH
生成的库文件输出路径
## 3.3. CMAKE_C_FLAGS
## 3.4. CMAKE_CXX_FLAGS
```cmake
CMAKE_CXX_FLAGS(-std=c++11)
CMAKE_CXX_FLAGS(-g)
```
## 3.5. CMAKE_SOURCE_DIR
代表工程顶层目录。
# 4. 一个例子
mydemo目录下是一个使用cmake的例子，运行：
```sh
cd mydemo
sh build-and-run.sh
```