《深度探索C++对象模型》 学习笔记
<!-- TOC -->

- [1. About Object](#1-about-object)
    - [1.1. C++对象模型](#11-c对象模型)
        - [1.1.1. 类](#111-类)
            - [1.1.1.1. 数据抽象](#1111-数据抽象)
            - [1.1.1.2. 继承](#1112-继承)
            - [1.1.1.3. 多态](#1113-多态)
        - [1.1.2. 理解虚函数](#112-理解虚函数)
- [2. The Semantics of Constructors](#2-the-semantics-of-constructors)
- [3. The Semantics of Data](#3-the-semantics-of-data)
- [4. The Semantics of Function](#4-the-semantics-of-function)
- [5. Semantics of Construction,Destruction,and Copy](#5-semantics-of-constructiondestructionand-copy)
- [6. Runtime Semantics](#6-runtime-semantics)
- [7. One the Cusp of the Object Model](#7-one-the-cusp-of-the-object-model)

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

### 1.1.2. 理解虚函数




# 2. The Semantics of Constructors
# 3. The Semantics of Data
# 4. The Semantics of Function
# 5. Semantics of Construction,Destruction,and Copy
# 6. Runtime Semantics
# 7. One the Cusp of the Object Model

