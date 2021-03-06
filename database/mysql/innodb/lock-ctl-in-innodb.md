InnoDB中的并发控制
===


## 隔离级别

隔离级别用于描述事务之间的隔离型:不同事务间在统一时间的操作是不是互相可见的,如果可见,可见到什么程度.

为了保证事务隔离型,必须要解决这几个问题:
* 脏读
	事务A正在修改a记录,事务B正好读取了A修改后的a记录,但是事务A随后发生了回滚操作,导致a的修改被撤销.这时,B读到的就是脏数据,脏数据可能会给应用程序带来错误的输入.
* 不可重复读
	事务A正在读取a记录,读完之后并没有立即结束事物A,于此同时B事务在修改a记录.B修改后,A再次读取a记录,发现a记录被修改了,两次读取相同数据的结果不一致.
* 幻读
	事物A正在读取某个范围内的记录,读完之后并没有立即结束事物A,于此同时B事务在这个范围内增加了一些记录.B增加完成后,A再次读取这个范围的记录,发现凭空多出了一些记录,就像幻觉一样.

一个好的数据库事务设计,应该充分解决上述三个问题,才能满足开发人员的需要.

根据是否解决了上述三个问题,SQL标准把事务的隔离级别分为了4级:

|隔离级别|脏读|不可重复读|幻读|
|:-:|:-:|:-:|:-:|
|ReadUncommited|no|no|no|
|ReadConmmited|yes|no|no|
|RepeatableRead|yes|yes|no|
|Serilization|yes|yes|yes|

InnoDB引擎实现了RepeatableRead,并通过多版本并发控制解决了幻读问题.

## InnoDB支持的粒度锁

InnoDB支持多粒度的锁定,以支持不同事务对一张表的不同部分进行修改.InnoDB支持4种粒度的锁:

|锁类型|简单解释|
|-|-|
|X锁|行级别的写锁,表示**要修改某一行记录**|
|S锁|行级别的读锁,表示**要读取某一行记录**|
|IX锁|表级别的写锁,表示**可能要修改表里的多条记录**|
|IS锁|表级别的读锁,表示**可能要读取表里的多条记录**|

注意,IX和IS属于一种意向锁,它只是表示,事务**可能要加锁**,并不是完全的拿到了锁.这代表,如果一个事务对表A加了IX锁,对A中的a记录加了X锁,另一个事务仍然可以对A加IX锁,但不能对a记录加任何锁,因为前一个事务已经明确表示**要修改a记录**.


## 行级锁

前面已经介绍了行级锁,为什么还要介绍一边呢?

因为InnoDB中,对一行记录加锁并没有那么简单,必须考虑范围问题.比如,对一行记录加锁,需不需要对前一行也加锁?需不需要对前一行和当前行中间的范围加锁?

有3种行锁:

|-|-|-|
|-|-|-|
|record lock|单个索引记录上的锁|如果是修改一行具体的数据,就需要加这样的行锁|
|gap lock|锁定一个范围,不包含记录本身|比如有两条索引记录a和b,gap锁就锁定(a,b)这个范围|
|next-key lock|锁定一个范围,包含记录本身|比如有两条索引记录a和b,gap锁就锁定(a,b]这个范围|

## 自增锁

如果表的主键是采用默认的自增主键,那么对表进行插入操作就需要处理自增的问题.自增的本质就是对一个计数器的增加操作,如果有多个事务同时在进行插入操作,必然会尝试增加计数器,那么就需要一种同步控制,也就是自增锁.自增锁与其他锁最明显的差别是,自增锁不必等待事务完成才释放,而是在insert语句完成后才释放.

## 加锁过程

这里太复杂了,暂时没有学会.可以参考这些文章:
* [mysql insert锁机制](http://yeshaoting.cn/article/database/mysql%20insert%E9%94%81%E6%9C%BA%E5%88%B6/)
* [MySQL 加锁处理分析](http://blog.sae.sina.com.cn/archives/2127)

## MVCC机制

虽然前文提到的各种天花乱坠的锁机制,可以保证事务的并发控制,但是仍存在效率低下的问题.MVCC就是解决这个问题的机制:多版本并发控制.

在可重复读和读已提交的事务隔离级别下,默认是采用MVCC机制,而不是加锁.

我很早就在想,Mysql中,多个事务同时进行,为什么读到的数据总是一样的?是不是有种像git一样的逻辑,给每个事务一个独立的备份去操作.

虽然这个问题本身是有错误的,但确实和MVCC有点像.这里简单记录一下MVCC原理.

MVCC依赖于记录的两个字段:创建时间,删除时间.

|字段|意义|
|-|-|
|创建时间|创建本条记录的事务ID|
|删除时间|删除本条记录的事务ID|

每个Mysql中的事务,都按照开始时间先后获取一个递增的事务ID.对于增删改查四种操作,他们这样处理:
* insert
	新增一条数据,并把当前事务ID更新到新增记录的创建时间
* delete
	把当前事务ID更新到待删除记录的删除时间
* update
	创建新的记录.把当前事务ID更新到**原记录的删除时间**和**新记录的创建时间**,
* select
	先走索引查找,或者是全局扫描.找到记录后,检查当前事务的ID是否在记录的[创建时间,删除时间)区间内,如果条件成立,这条记录将被返回给select语句.


应该还有一个后台逻辑,这个逻辑检查当前存在的事务,如果某些记录不存在任何正在访问的事务,并且是过期数据,应该会被删掉.暂时还没有看到这部分的资料,先不研究了.