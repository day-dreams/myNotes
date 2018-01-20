# C++中的内存控制函数

这不是一片深入cpp内存控制的笔记,仅仅是记录一些以前没有理解透彻的概念.

<!-- TOC -->

- [C++中的内存控制函数](#c中的内存控制函数)
    - [malloc,calloc,realloc,free](#malloccallocreallocfree)
        - [malloc](#malloc)
        - [calloc](#calloc)
        - [realloc](#realloc)
        - [free](#free)
            - [为什么free不需要提供内存大小?](#为什么free不需要提供内存大小)
    - [operator new,delete](#operator-newdelete)
        - [为什么要有operator new和operator delete的重载功能?](#为什么要有operator-new和operator-delete的重载功能)
    - [new,delete expression](#newdelete-expression)

<!-- /TOC -->


## malloc,calloc,realloc,free

这是一套C语言的内存管理函数,在C++中应该尽量不使用.

### malloc

```cpp
void* malloc( std::size_t size );
```

malloc仅仅是从堆上分配出一份指定大小的内存,并不执行任何初始化或者构造函数.

### calloc

```cpp
void* calloc( std::size_t num, std::size_t size );
```

calloc从堆上分配出一个数组，数组有num个元素,每个元素占size bytes.并且,这块分配出来的内存会被初始化,每个bytes都被初始化为0.

### realloc

```cpp
void* realloc( void* ptr, std::size_t new_size );
```

realloc先释放掉原有的内存,再开辟新的内存返回回来.要求释放掉的内存必须是之前malloc或则calloc产生的内存.

注意,如果新开辟的内存大小小于原来的内存,可能直接会被实现为:进丢弃多余的内存大小,内存地址不变.

### free

释放之前由malloc,calloc,realloc分配的内存.

#### 为什么free不需要提供内存大小?

malloc等函数会在底层实现里记录下分配的内存大小,free直接使用那个大小就行.

```cpp
```

## operator new,delete

```cpp
void* operator new  ( std::size_t count );
void* operator new[]( std::size_t count );
void operator delete  ( void* ptr );
void operator delete[]( void* ptr );
```
operator new和operator delete就是cpp版本的malloc和free,在cpp中应该避免使用c风格的malloc和free.

使用方式:
```cpp
auto ptr = operator new(10);
operator delete(ptr);
```

### 为什么要有operator new和operator delete的重载功能?

这两个都只不过是函数,并且可以被重载.这样的功能可以用来实现自己定制的内存分配释放策略.

但是,全局的operator new/delete重载侵略行特别强,它直接替换了程序里使用到的所有库的内存分配策略.

[这篇博客](http://blog.csdn.net/solstice/article/details/6198937)有详细的说明.

另外,[这篇博客](http://blog.csdn.net/Solstice/article/details/4401382)解释了为什么STL的allocator是一种无用的设计.

## new,delete expression

```cpp
auto x = new Object;
delete x;

auto y = new Object[10];
delete y;//error! 内存泄漏
delete[] y;
```

new expression会先使用operator new来分配内存,再调用构造函数来初始化之.delete expression会先使用operator elete来释放内存,再调用析构函数来初始化之.[这里](./test.cpp)有示例代码,可以观察到new/delete expression调用new/delete operator的过程.

