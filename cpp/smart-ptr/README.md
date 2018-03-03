C++ 智能指针
===


<!-- TOC -->

- [解决问题](#解决问题)
- [unique_ptr](#unique_ptr)
- [shared_ptr](#shared_ptr)
	- [实现细节](#实现细节)
	- [线程安全](#线程安全)
	- [make_shared vs shared_ptr<T>()](#make_shared-vs-shared_ptrt)
- [weak_ptr](#weak_ptr)
	- [用途一:解决循环引用](#用途一解决循环引用)
	- [用途二:用于缓存](#用途二用于缓存)
- [注意](#注意)
- [reference](#reference)

<!-- /TOC -->

## 解决问题

裸指针的问题很突出:
* 对象所有权不明显,容易造成忘记释放或二次释放
* 无法确认指针是否合法
* 需要显式delete,如果delete之前出抛出了异常,就会导致资源泄漏



## unique_ptr

`unique_ptr`具有对资源的所有权和独占权.当`unique_ptr`被销毁,占有的资源也会被销毁.


它的性质如下:
* **MoveConstructible and MoveAssignable**,但不是**CopyConstructible or CopyAssignable**
* 只有非const unique_ptr可以转移对象所有权
* 内部对象可以是一个对象,也可以是对象数组
* 使用deleter自动释放内存,deleter可以在构造函数里给出,默认为`delete expression`

## shared_ptr

`shared_ptr`是带有引用计数的智能指针,允许多个智能指针引用同一个对象.

它的性质如下:
* **CopyConstructible and CopyAssignable**
* 可以使用自己的deleter
* 复制操作满足:std::atomic::fetch_add,std::memory_order_relaxed 

### 实现细节

由两部分组成:
* 指向对象的指针
* 指向控制块的指针
	* 指向被控制对象的指针,或者是被控制对象本身.
	* share数量的指针
	* weak指针的数量
	* allocator
	* deleter

大致如下图:

![inside sptr](inside-smartptr.png)

### 线程安全

shared_ptr并不是严格意义上的线程安全,它只保证:
* std::atomic::fetch_add, 增加引用计数是原子性的
* std::memory_order_relaxed, 多个线程同时增加引用计数时,只保证原子性,不保证顺序

所以,很多针对shared_ptr的操作都不是线程安全的:
* 减小引用计数. 按照发生顺序分为减小user_count不安全, 调用析构函数不安全, 释放内存不安全.
* 任何读写内部对象的操作.

### make_shared vs shared_ptr<T>()

```cpp
shared_ptr<int> sptr1(new int(0));//多次次分配内存,控制块和裸指针不在一起
auto sptr2=make_shared<T>(0);//一次分配内存,控制块和裸指针在一起
```

使用make_shared可能获得这样的内存布局:
![](make-shared.png)

由于new内部调用了malloc,一次new的开销是无法忽略的.**理论上make_shared更快**.

## weak_ptr

weak_ptr是一种弱指针,它不具有对象的所有权,而是作为一个类似helper的东西,帮助判断真正的对象是不是还有效.

```cpp

shared_ptr<int> ptr(1);
weak_ptr<int> wptr=ptr; 

ptr.reset();//ptr释放了管理的对象

if (wptr.lock()){//如果这个对象还存在,即ptr没有被reset
	//access it 
}else{
	//do nothing
}
```

这才真正意义上解决了:**原生指针无法判断自己是不是个悬浮指针(dangling pointer)**.



### 用途一:解决循环引用


现在有team和member两个类,并且互相维护一直指向对方的指针.程序运行期间,team和member都可能被销毁了.

如果采用原生指针,就不知道彼此是不是被销毁了,必须程序员自己去控制.这种做法不利于程序维护,毕竟谁都有个忘了的时候.

如果采用shared_ptr,team和member互相保持引用.如果试图使用shared_ptr的member function来释放一个team,却发现发现team还被member的shared_ptr引用,所以不能释放内存,只减小引用计数;如果试图释放member,效果也一样.这样造成了循环引用,对象永远无法消除,产生了**内存泄漏**.

解决方案是使用weak_ptr,weak_ptr提供对象是否存在的检查,却没有对象所有权.只要member和team都以weak_ptr的方式来引用彼此,就不会产生循环引用的问题.

### 用途二:用于缓存

容易理解,缓存里全部是weak_ptr,如果对象真的被销毁了,weak_ptr可以观察到;如果没有,weak_ptr由可以生成shared_ptr,提供所有权.


## 注意

* CopyConstructible, CopyAssignable
* MoveConstructible, MoveAssignable
* std::atomic::fetch_add with std::memory_order_relaxed 
* get()方法返回裸指针,并且不影响原智能指针的性质,应该尽量避免使用
* 使用智能指针必须进行额外的同步控制
* 几乎所有智能指针都没有保证完整的线程安全,可以使用std::atomic_exchange或额外的同步控制等,来保证线程安全
* shared_ptr的reset函数,只调整当前shared_ptr的所有权;如果被管理的对象引用计数为0,则会释放之
* 理论上,make_shared比shared_ptr<T>()快,可以作为性能优化的一个方面;但实际上,不优化时make_shared要慢,所以还是得根据优化等级来判断

## reference

[陈硕: 为什么多线程读写 shared_ptr 要加锁？](http://blog.csdn.net/solstice/article/details/8547547)

[cppreference:std::shared_ptr](http://en.cppreference.com/w/cpp/memory/shared_ptr)

[stackoverflow:when weak_ptr is used](https://stackoverflow.com/questions/12030650/when-is-stdweak-ptr-useful)