C++的对齐策略
===

<!-- TOC -->

- [什么是对齐](#什么是对齐)
- [什么时候发生对齐](#什么时候发生对齐)
	- [class/struct定义](#classstruct定义)
	- [对象定义](#对象定义)
- [注意的地方](#注意的地方)
- [reference](#reference)

<!-- /TOC -->

## 什么是对齐

对齐(align)是指在对象间填充一定的字节,从而让对象从某种特定地址开始.

回忆8086cpu的访存特点:读偶数地址开始的16个字节,比读奇数地址开始的16个字节要快.这是内存结构决定的:8086内存存储提是低位交叉的.


## 什么时候发生对齐

两种情况.

### class/struct定义

基本原则是:
* class/struct每个成员相对于起始地址的偏移能够被其自身大小整除，如果不能则在前一个成员后面补充字节
* class/struct总体大小能够被最宽的成员的大小整除，如不能则在后面补充字节

这是一个例子:

```c
struct mystruct1{
	// sizeof(mystruct1)==20
	char  a;//1 bytes, 7 bytes for padding
	char* b;//8 bytes
	int   c;//4 bytes
}

//优化空间后
struct mystruct2{
	// sizeof(mystruct2)==16
	char* b;//8 bytes
	int   c;//4 bytes
	char  a;//1 bytes
}
```


### 对象定义

对齐不仅发生在class/struct定义时,还会发生在一般的变量定义里.一般变量的对齐规则比较复杂,是编译器平台相关的,还**可能发生变量重排列**.这里列举一种规则如下:

>除char外，所有其他类型都有“对齐要求”：char可起始于任意字节地址，2字节的short必须从偶数字节地址开始，4字节的int或float必须从能被4整除的地址开始，8比特的long和double必须从能被8整除的地址开始。

考虑下面的变量定义:

```c
char *p;//8 bytes
char c;//4 bytes
int x;//4 bytes
```

他们实际上分别占了8,4,4字节,```char c```被填充了3字节的内存空洞.

如果想减小内存空间使用,可以这样声明:
```c
char *p;//8 bytes
int x;//4 bytes
char c;//1 bytes
```

## 注意的地方

* 对齐可能会重排列变量的布局

* 如果追求极致的空间消耗,可以考虑优化对齐.

## reference

[失传的C结构体打包技艺](https://github.com/ludx/The-Lost-Art-of-C-Structure-Packing)