C++ 的Storage class specifiers
===

<!-- TOC -->

- [storage duration](#storage-duration)
	- [automatic storage duration](#automatic-storage-duration)
	- [static storage duration](#static-storage-duration)
		- [global variables](#global-variables)
		- [static local variables](#static-local-variables)
	- [thread storage duration](#thread-storage-duration)
	- [dynamic storage duration](#dynamic-storage-duration)
- [linkage](#linkage)
	- [no linkage](#no-linkage)
	- [internal linkage](#internal-linkage)
	- [external linkage](#external-linkage)
- [不同类型变量的初始化顺序](#不同类型变量的初始化顺序)

<!-- /TOC -->

C++ 的Storage class specifiers用于控制变量的两种属性:
* 存储周期
* 链接方式

主要包含这几个关键字:
* auto
* static
* extern
* thread_local

这些关键字对应的存储周期和链接方式如下:

|关键字|存储周期|链接方式|
|-|-|-|
|auto|automatic storage duration|不存在链接,是函数本地使用的临时变量|
|static|static or thread storage duration ||
|extern|static or thread storage duration ||
|thread_local|thread storage duration|不存在链接|


## storage duration

### automatic storage duration

在一个{}中,任何没有关键字修饰的变量都具有自动存储周期,并且是在栈上分配.随代码块的进入而创建,退出而销毁.

### static storage duration

有两种变量具有static storage duration:
* global variables,即在某个namespace或全局namespace声明的变量
* local variables,在代码块内使用static或者extern修饰的变量

#### global variables

这样的变量在程序执行之前被分配,即main()执行之前;在程序退出的时候被析构.

并且按照定义的先后顺序初始化,逆序销毁.


#### static local variables


程序首次进入这个代码块,执行到这一句定义时,进行初始化;程序退出时销毁.

如果初始化时抛出了异常,那么每次执行到这一句定义时,都要初始化,直到初始化成功.


### thread storage duration

这种存储周期是跟随线程的.每个线程都有一份单独的变量,线程第一次使用这个变量时初始化,退出时销毁.

注意,如果thread_local是在代码快中定义的,算是一种使用;如果实在全局namespace中定义,不算做使用.

### dynamic storage duration

被new出来的变量.

## linkage

### no linkage

**同一代码块内的代码可以访问**.一般变量都是no linkage的.

### internal linkage

**同一编译单元的代码可以访问**.


### external linkage

不同编译单元的代码,甚至不同语言之间都可以访问.但是只能有一个定义.


## 不同类型变量的初始化顺序

不考虑类成员变量,在同一个编译单元内,顺序如下:

1. global variables  
	无论有没有static修饰,都是第一个初始化,并且按照定义的先后顺序初始化.**static对于global variables的效果只是限制其他编译单元的访问**.
2. static local variables  
	代码首次执行到这句定义时构造.如果构造失败(构造函数抛出了异常),会在下次执行时再次构造,直到成功.
3. thread_local variables  
	如果使用了thread_local变量,则先构造;否则thread_local不构造.