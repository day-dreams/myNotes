深入理解Golang slice
===

很多语言层面的API问题，都可以通过深入源码的方式来解决。这样最为直接，也最为让人信服。这篇文章将从源码层次，分析Golang中slice的实现方式，及相关的函数调用，最后分析slice常见的坑。

## 源代码slice.go

这个源码实现了一些slice的内部函数,从[源码]()上看，slice的本质和c++中的vector非常像，一看就知道是怎么回事：

```golang
type slice struct {
	array unsafe.Pointer
	len   int
	cap   int
}
```

下面介绍源码中相关的函数调用.

### makeslice()

    `makeslice()`负责slice的创建工作，它首先检查申请的slice大小是否合法，完成后再分配相关的内存，用这块内存的指针、容量、元素个数来初始化slice结构体，最后返回。

```golang
func makeslice(et *_type, len, cap int) slice {

    //检查
	maxElements := maxSliceCap(et.size)
	if len < 0 || uintptr(len) > maxElements {
		panic(errorString("makeslice: len out of range"))
	}

	if cap < len || uintptr(cap) > maxElements {
		panic(errorString("makeslice: cap out of range"))
	}

	//经过一系列检查后，分配内存，并初始化新的slice结构体返回
	p := mallocgc(et.size*uintptr(cap), et, true)
	return slice{p, len, cap}
}
```

### growslice()

`growslice()`负责`append()`期间的slice扩容工作,它会创建新的内存,并把旧元素复制过去.

一般是2倍扩容;特殊情况下,如果申请的新容量不足原容量的两倍,并且原slice的元素大于等于1024个,那么新容量会略大于申请的新容量.cap的增长表现可以概括为:**先指数级,再放缓**.

值得一提的是,grouslice采取的复制函数,即`memmove`,它直接是汇编实现的,而不是go语言编译成汇编代码,所以说golang还是很高效的.

### slicecopy()

`slicecopy()`就是简单的元素复制,会自己判断复制元素的个数,**不会引起任何新的内存分配**.


## append,copy

与前一节提到的函数不同,`append()`、`copy()`等build-in函数不在源码包里，他们是编译器实现的。编译器维护了一份内建函数的实现，在编译代码中的内建函数时，直接使用自己的版本去翻译，而不用去走一整套编译流程。

源码里只能找到这些内建函数的签名和注释。

### append

```golang
// The append built-in function appends elements to the end of a slice. If
// it has sufficient capacity, the destination is resliced to accommodate the
// new elements. If it does not, a new underlying array will be allocated.
// Append returns the updated slice. It is therefore necessary to store the
// result of append, often in the variable holding the slice itself:
//	slice = append(slice, elem1, elem2)
//	slice = append(slice, anotherSlice...)
// As a special case, it is legal to append a string to a byte slice, like this:
//	slice = append([]byte("hello "), "world"...)
func append(slice []Type, elems ...Type) []Type
```

可以看到:
* 如果slice的容量不够，会引起底层数组的更新。
* 允许向`[]char`中apend`string`。


由于slice底层数组内存管理走的是gc机制，当append引起了数组扩容，如果没有其他的slice变量还指向旧数组，可能会触发内存回收;否则，原数组的内存依然存在。


### copy

```golang
// The copy built-in function copies elements from a source slice into a
// destination slice. (As a special case, it also will copy bytes from a
// string to a slice of bytes.) The source and destination may overlap. Copy
// returns the number of elements copied, which will be the minimum of
// len(src) and len(dst).
func copy(dst, src []Type) int
```

copy相对简单，不会引起内存的分配。


## make

针对slice的make调用很容易理解，应该是直接调用`slice.go`中的`makeslice()`函数。可以放心使用，不仅存在任何坑。

## 一些坑

* `[low:high]`操作

这个操作用于取一个slice的subslice。**它创建了一个新的slice，但并没有创建新的底层数组**。所以slice和subslice指向的是同一个底层数组，区别是彼此的cap、len和底层数组首地址不同。**在没有扩容发生时，二者对于元素的修改会影响到彼此**。

* `[low:high:max]`操作

这个操作用于取一个slice的subslice。值得注意的是，他把新slice的cap设置为max-low，底层数组依然没有变。

这个操作有什么用呢？试想，如果我们把max和high取相同的值，那么新slice的cap和len就相等了。后续如果先对新slice进行append，那么一定会触发底层数组的重新分配，就不会影响到原来的slice了。但是，如果只是修改元素值，还是会影响到原slice。

根据一个slice创建一个新slice最好的方法就是先调用make，再调用append，这样新旧slice的操作就不会相互影响了。


## reference

[Go Slices: usage and internals](https://blog.golang.org/go-slices-usage-and-internals)
[Go语言slice的那些坑](https://blog.csdn.net/zhanchenxing/article/details/50157583)