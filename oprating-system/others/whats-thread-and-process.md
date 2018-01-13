线程及进程详解

这篇文章主要是为了探讨,进程和线程到底是什么,以及Linux是如何他们的.

<!-- TOC -->

- [1. POSIX概念上的线程和进程](#1-posix概念上的线程和进程)
    - [1.1. Concepts:process,thread](#11-conceptsprocessthread)
    - [1.2. 多线程模型](#12-多线程模型)
        - [1.2.1. Many-to-One](#121-many-to-one)
        - [1.2.2. One-to-One](#122-one-to-one)
        - [1.2.3. Many-to-Many](#123-many-to-many)
        - [1.2.4. Two-Levels](#124-two-levels)
    - [1.3. 隐式线程](#13-隐式线程)
        - [1.3.1. Thread Pool](#131-thread-pool)
        - [1.3.2. OpenMP](#132-openmp)
        - [1.3.3. Grand Central Dispatch](#133-grand-central-dispatch)
    - [1.4. 线程带来的问题](#14-线程带来的问题)
        - [1.4.1. fork的两种模式](#141-fork的两种模式)
        - [1.4.2. 信号处理](#142-信号处理)
        - [1.4.3. 线程撤销](#143-线程撤销)
        - [1.4.4. Thread-Local Storage与内存泄漏](#144-thread-local-storage与内存泄漏)
        - [1.4.5. Scheduler Activations](#145-scheduler-activations)
- [2. Linux实现下的线程和进程(注意和man page结合)](#2-linux实现下的线程和进程注意和man-page结合)
    - [2.1. 为什么要有LWP,Process,Thread?](#21-为什么要有lwpprocessthread)
    - [2.2. Thread ID和PID](#22-thread-id和pid)
    - [2.3. 进程数据结构](#23-进程数据结构)
    - [2.4. 进程间的关系](#24-进程间的关系)
    - [2.5. LWP的创建](#25-lwp的创建)
    - [2.6. 信号的diliver机制(进程组)](#26-信号的diliver机制进程组)
    - [2.7. 内核线程](#27-内核线程)
    - [2.8. 进程切换](#28-进程切换)
    - [2.9. 相关的API](#29-相关的api)
        - [2.9.1. fork,exec](#291-forkexec)
        - [2.9.2. clone](#292-clone)
- [3. 参考链接](#3-参考链接)

<!-- /TOC -->

# 1. POSIX概念上的线程和进程

## 1.1. Concepts:process,thread

## 1.2. 多线程模型

### 1.2.1. Many-to-One

### 1.2.2. One-to-One

### 1.2.3. Many-to-Many

### 1.2.4. Two-Levels

## 1.3. 隐式线程

### 1.3.1. Thread Pool

### 1.3.2. OpenMP

### 1.3.3. Grand Central Dispatch

## 1.4. 线程带来的问题

### 1.4.1. fork的两种模式

### 1.4.2. 信号处理

### 1.4.3. 线程撤销

### 1.4.4. Thread-Local Storage与内存泄漏

### 1.4.5. Scheduler Activations

# 2. Linux实现下的线程和进程(注意和man page结合)

## 2.1. 为什么要有LWP,Process,Thread?

早期linux内核之支持用户级线程,cpu调度单位是进程,会带来几个问题:  
* 线程数越多,进程获得的cpu时间越多  
* 在一个进程内,只要有一个线程阻塞,其他线程也不得不阻塞

所以,linux使用了LWP的概念,在用户级线程和内核级线程(即内核代码中的**struct_task**)间加入了一层抽象:Lightweight Process.

每个用户级线程对应一个轻量级进程.LWP带来了几点好处:  
* 支持不同级别的task间资源共享  
    不同LWP可以共享某些资源,如代码,文件描述符,堆内存等  
* 带来了更强的调度灵活性  
    一个进程内的线程也可以并发运行
    
## 2.2. Thread ID和PID

## 2.3. 进程数据结构

## 2.4. 进程间的关系

## 2.5. LWP的创建

## 2.6. 信号的diliver机制(进程组)

## 2.7. 内核线程

## 2.8. 进程切换

## 2.9. 相关的API

### 2.9.1. fork,exec

### 2.9.2. clone


# 3. 参考链接

![What are Linux Processes, Threads, Light Weight Processes, and Process State](http://www.thegeekstuff.com/2013/11/linux-process-and-threads/)