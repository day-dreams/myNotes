# redis中的数据类型

<!-- TOC -->

- [redis中的数据类型](#redis中的数据类型)
    - [Strings](#strings)
    - [Lists](#lists)
        - [相关命令关键字](#相关命令关键字)
    - [Hashes](#hashes)
    - [Sets](#sets)
    - [Bitmaps](#bitmaps)
    - [HyperLogLogs](#hyperloglogs)

<!-- /TOC -->

## Strings

作为value，String适合存储html网页、图片甚至是二进制数据，最大为512MB。

## Lists

本质是链表，而不是python中的数组list。适用于保存社交网络中用户最近的上传，进程间通信(生产消费模型)等场景。

### 相关命令关键字
* LPUSH
* RPUSH
* LRANGE
    返回部分元素
* LTRIM
    返回部分元素并删除其他元素
* RPOP
    弹出元素，list为空时**不阻塞**(消费者调用RPOP，生产者则简单调用LPUSH)
* BRPOP
    弹出元素，list为空时**阻塞**(消费者调用RPOP，生产者则简单调用LPUSH)

## Hashes
就是一个dict。
```shell
> hmset user:1000 username antirez birthyear 1977 verified 1
OK
> hget user:1000 username
"antirez"
> hget user:1000 birthyear
"1977"
> hgetall user:1000
1) "username"
2) "antirez"
3) "birthyear"
4) "1977"
5) "verified"
6) "1"
```

## Sets
```shell
> sadd myset 1 2 3
(integer) 3
> smembers myset
1. 3
2. 1
3. 2
```

## Bitmaps

## HyperLogLogs