《深度探索C++对象模型》 学习笔记
<!-- TOC -->

- [1. About Object](#1-about-object)
    - [1.1. C++对象模型](#11-c对象模型)
        - [1.1.1. 类](#111-类)
            - [1.1.1.1. 数据抽象](#1111-数据抽象)
            - [1.1.1.2. 继承](#1112-继承)
            - [1.1.1.3. 多态](#1113-多态)
        - [1.1.2. 对象的内存布局](#112-对象的内存布局)
            - [无虚函数的对象内存布局](#无虚函数的对象内存布局)
                - [简单类](#简单类)
            - [简单类的派生类](#简单类的派生类)
            - [带虚函数的对象内存布局](#带虚函数的对象内存布局)
- [2. The Semantics of Constructors](#2-the-semantics-of-constructors)
    - [Default Constructors](#default-constructors)
    - [Copy Constructors](#copy-constructors)
        - [Memberwise Initialization, Bitwise Copy和Copy Constructor](#memberwise-initialization-bitwise-copy和copy-constructor)
        - [Copy construcotr如何设定vptr](#copy-construcotr如何设定vptr)
- [3. The Semantics of Data](#3-the-semantics-of-data)
- [4. The Semantics of Function](#4-the-semantics-of-function)
- [5. Semantics of Construction,Destruction,and Copy](#5-semantics-of-constructiondestructionand-copy)
- [6. Runtime Semantics](#6-runtime-semantics)
- [7. One the Cusp of the Object Model](#7-one-the-cusp-of-the-object-model)
- [8. Reference](#8-reference)

<!-- /TOC -->
# 1. About Object
## 1.1. C++对象模型
### 1.1.1. 类
类这一概念是为了加强 数据单元和操作方法 的联系。它规定了数据如何组织成一个单元，以及哪些操作可以作用在数据单元的哪些成员上。  
在C语言中，代码逻辑是过程式的。组织的函数越多，越会发现难以组织、操作函数，因为你只能依靠 函数名、参数列表、返回值 来决定作用到当前数据的 函数。随着数据的复杂化，特别是出现struct嵌套时，你不知道哪些函数是为了这个struct而设计，而不是为了其包含的另一个struct设计的。虽然可以使用函数指针来解决这个问题，但带来了额外的空间使用。另一方面，C的struct结构没有共有/私有的概念，无法隐藏某些数据成员，结果是给代码编写带来负担。  
类就是为了解决这些问题出现的，它带来的特性有：数据抽象，继承和动态绑定。  
#### 1.1.1.1. 数据抽象
数据抽象就是将数据和对应的操作方法抽象出来，程序员只需要知道一个类对象有哪些数据和方法可以访问（public data/function members），而不用关心它们如何实现。类的设计者将**接口**暴露给类的使用者，将接口的**实现**隐藏起来，自己负责。数据抽象通过接口的实现和使用分离，直接加强了代码复用。  
#### 1.1.1.2. 继承
数据抽象注重的是代码复用，继承则更注重代码扩展。继承（inheritance）在原有代码的基础上，允许程序员对数据和操作进行扩展。程序员可以在原有的代码基础上，组织出自己的逻辑。比如，```class Point2D```提供了二维空间上点的一些操作，如两点之间的距离，```class Point3D```继承了```class Point2D```，不仅提供两点投影到某个坐标平面上的距离，还提供两点在空间上的距离。这种扩展可以看作是逻辑分层，如  ```class Queue```提供FIFO功能，```class TaskQueue:public Queue```则提供线程安全特性。
#### 1.1.1.3. 多态
多态允许程序无视对象所属类别的差异，而调用这些类共有的成员。多态的好处在于，给予程序更多的灵活性，调用的函数版本根据具体的类型来确定。例如：

```cpp
class Tigger:public Mammal;
class Cat:public Mammal;
virtual Mammal::giveBaby();

Mammal* p2tigger=new Tigger();
Mammal* p2cat=new Cat();

p2tigger->giveBaby();//编译器并不知道p2tigger指向mammal还是tigger，直到运行时才会根据虚函数指针来决议。
p2cat->giveBaby();//同上
```

C++中多态的实现依赖虚函数,《c++ primer》这样解释：
> 当我们使用基类的引用/指针去调用基类中定义的函数时，我们并不知道该函数真正作用的对象是什么类型，因为它可能是一个基类的对象，也可能是一个派生类的对象。如果该函数是虚函数，则知道运行时才会决定到底执行哪个版本，判断的依据是引用/指针所绑定的对象的**真实类型**。

也就是说，在代码真正运行之前，必须决定使用引用/指针调用的函数到底是哪个版本。
|函数类型|绑定时期|绑定版本|
|-|-|-|
|基类中的普通函数|编译期|基类版本|
|派生类中的普通函数|编译期|派生类版本|
|虚函数|运行时|根据对象的真实类型来决定绑定版本|

### 1.1.2. 对象的内存布局

#### 无虚函数的对象内存布局

只要没有虚函数,类的对象布局其实很简单.与c语言相比,这种情况下的类不但没有增加性能负担,反而提高了编程时的便捷.
##### 简单类

简单类没有任何继承关系,也没有虚函数，只含有基本的成员函数和成员对象．  
* 成员函数实际上是一种语法糖，虽然看起来是**在运行期用类对象来调用类的方法**，实际是**在编译期将指向类对象的指针(隐含的this指针)传给类的函数进行调用**  
* 成员变量则类似于传统C语言的struct,一般编译器保证按照在类声明中成员变量出现的顺序去安排内存布局,使成员变量紧凑排列(如果需要,会进行**对齐**)    
而对于类的静态成员变量和静态成员函数,它们都被放在单独的区域,不在个别的对象中,调用和访问都是通过指针实现.

#### 简单类的派生类
简单类的派生情况,不过是将子类的成员变量加到父类的成员变量之后(同样需要考虑对齐).不论是单重继承还是多重继承,都可以通过粗暴的指针操作进行成员变量访问,具体细节见[代码](https://github.com/day-dreams/myNotes/tree/master/C%2B%2B/easy-class-inheritance.cpp).  

#### 带虚函数的对象内存布局
虚函数是由vptr实现的,它的特性有:
vptr指向一个虚函数表格,表格里是**指向虚函数和type_info object的指针**  
* type_info object用于实现RTTI(runtime type identification),主要用在类型转换和typeid()  
* vptr的设定和重置由constructor,destructor,copy constructor完成  
* vptr一般被放在对象内存布局的最前方  

总结,带虚函数的对象的内存布局为:vptr+成员变量,验证见[代码](https://github.com/day-dreams/myNotes/tree/master/C%2B%2B/vfun-class-model.cpp).注意,我的例子**无法观察到虚函数表的结尾标志**.  
    
<!---->
# 2. The Semantics of Constructors

## Default Constructors

现在来讨论一下构造函数.一般地,我们应该为类设计一个Default Constructor,防止某些情况下我们确实需要它,而编译器又没有为我们合成.那么什么情况下编译器会为我们合成默认构造函数呢?

首先,如果我们没有提供任何形式的构造函数,编译器会弄出一个implicit-defined default constructor.注意,这个默认构造函数还没有被定义!也就是没有函数实体.

接下来,如果编译器发现,implicit-defined default constructor没有被删除,并且有需要使用这个函数的地方,就会实现这个函数,也就是合成implicitly-defined default constructor.**这个合成的默认构造函数,和一个被程序员显示定义的,没有参数列表,函数体空的默认构造函数,效果完全相同**.

我们可以清晰的指出, **implicitly-defined default constructor不一定会被合成出来,除非它是odr-used**. 对于函数来说,odr-used代表,程序的某个地方需要对这个函数进行取地址,或者是这个函数被调用了.这里的implicitl-defined default constructor被调用只有一种可能:编译系需要!编译器需要调用他,来执行一定的操作(执行member class或者parent class的默认构造函数等).一但被需要,就意味着要有函数实体,编译器会把一些操作插入这个函数实体.

更进一步,一个类会被编译器合成出默认构造函数,如果下列有一条成立:
* 这个类的某个member class object,具有default constructor
* 这个类的基类,具有default constructor
* 这个类有一个virtual function
* 这个类有一个virtual base function

但是,默认构造函数的真正作用是:
* 保证基类或者成员类的默认构造函数能被执行
* 设定虚指针(vptr)

## Copy Constructors

通常，我们定义一个copy constructor是处于深拷贝的目的.比如,一个类含有字符串指针(char*),如果我们想复制一份字符串,而不是复制一份指向字符串的指针,就必须自己定义copy constructor来完成这个任务.

如果我们没有定义copy constructor,编译器也有可能帮我们合成一个.合成的具体情况类似default constructor.

首先是Implicitly-declared copy constructor.如果我们没有定义任何拷贝构造函数,编译器会隐式声明一个,也就是Implicitly-declared copy constructor.但它的形式可能是T::T(const &T),也可能是T::T(T&),区别在于能不能捕获rvalue reference.这两种形式的Implicitly-declared copy constructor选择,也取决与基类,虚基类,非静态成员类的copy constructor情况.

然后,如果Implicitly-declared copy constructor不是被delete了,也不是一个trival的,并且是odr-used,编译器就会真正生成这个拷贝函数,也就是Implicitly-defined copy constructor.这种copy constructor的用途也是在于**调用子类,虚基类,成员类的copy constructor,或者初始化vptr.**

简单说,编译器会为我们生成copy constructor,当这个copy constructor不是trival的.

### Memberwise Initialization, Bitwise Copy和Copy Constructor

这三者是个什么关系呢?我们说,类对象的拷贝是memberwise的.也就是说,每个成员对象都会被拷贝,这个拷贝有两种方式:bitwise copy和通过copy constructor去拷贝. 

* bitwise copy就是按照成员的内存分布,一个byte一个byte去拷贝.对于一些简单的类,这样做很合适.这个类```class Point{double x;double y;double z;}```,它只需要这种bitwise的拷贝.

* copy constructor则不同,对于一般的成员,copy constructor也是进细腻个bitwise拷贝;但是对于一些**有需要**的成员,则会有一些**额外的操作**.

当出现下列任何一种情况,我们说是有需要的,编译器会生成一个copy constructor:
* 这个类的某个member class object,且这个成员类有copy constructor(无论是不是编译器生成的,既然存在,就说明有个这需要去调用他)
* 这个类的基类有copy constructor
* 这个类有虚函数,virtual function
* 这个类的继承链中,存在虚基类,virtual base class.

可以看出,**编译器生成copy constructor的作用是为了保证基类或成员类的copy constructor能被调用,或者是需要设定虚函数表**.这也是所谓的额外操作.

### Copy construcotr如何设定vptr



# 3. The Semantics of Data
# 4. The Semantics of Function
# 5. Semantics of Construction,Destruction,and Copy
# 6. Runtime Semantics
# 7. One the Cusp of the Object Model

# 8. Reference

[图说C++对象模型：对象内存布局详解](http://www.cnblogs.com/QG-whz/p/4909359.html)

[C++对象模型之内存布局一](http://luodw.cc/2015/10/06/Cplus1/)  
[C++对象模型之内存布局二](http://luodw.cc/2015/10/07/Cplus2/)
[C++对象模型之内存布局三](http://luodw.cc/2015/10/08/Cplus3/)

[C++中的RTTI机制解析](http://blog.csdn.net/three_bird/article/details/51479175)