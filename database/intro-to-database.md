# 数据库基本概念清单

## 一. 数据库和数据库管理系统  
* _database_  
  数据库就是保存有组织的数据的**容器**
* _DBMS_  
  数据库管理系统是帮助我们和这个容器打交道的软件

## 二. 表（table）
一种结构化的文件，可以用来存储某种特定类型的数据。
* 表名唯一
* 表内数据类型统一  

我们用**模式**（schema）来:
* 定义数据如何在表中存储
* 表之间的关系

表中的数据按照**行**来添加，每行数据的特征由列来定义。其中，每个列都有自己的数据类型，限制该列可以存储的数据类型。

* 主键（primary key）   
表中每一行数据都有自己的唯一标识符，即主键。
主键可以单独定义在表的一列上，但也可以分散成多个列。不管怎样，主键必须唯一且不为空。  

## 三. 关于DBMS
一般有两类：
* 基于共享文件系统，主要用作桌面用途。
* 基于C/S。  
MySQL就是一款基于CS的数据库管理系统。其中，服务器就是运行MySQL服务器软件的计算机，而客户的概念很抽象，可以是MySQL的UI、脚本、web开发语言、C\C++、Java等。我们主要使用三种客户机程序：
* mysql命令行程序
* MySQL Administrator
* MySQL Query Browser

## 四. 常用命令
* USE db_name : 切换数据库
* SHOW DATABASES : 返回可用数据库列表
* SHOW TABLES : 返回数据库表的列表信息
* SHOW COLUMNS FROM db_name : 显示db_name的列信息
* DESCRIBE db_name : 同上
* 创建表  
  ```sql
  CREATE TABLE table_name(
    id int not null auto_increment,
    name char(20) not null,
    sex char(4) not null,
    primary key (id)
  );
  ```
  ```sql
  create table analysis_ADNS(
  domain char(20),
  ns_ip char(15),
  detected int default 0,
  aa char,
  ra char,
  status char(10),
  answers_num int,
  authority_num int,
  additional_num int,
  answers tinytext,
  authority tinytext,
  additional tinytext,
  primary key(domain,ns_ip)
  );
  ```
## 五. 数据类型
* 枚举
  ```sql
  create table enum_test(
    id int auto_increment,
    type ENUM('A','SOF','CNAME','AAAA')
  );
  ```
* 变长字符串
  ```sql
  create table varchar_test(
    id int auto_increment,
    name VARCHAE(100)
  );
  ```

## 六. 几种重要的操作
 * SELECT
   ```sql
   SELECT * FROM table_name;
   SELECT id FROM table_name;
   SELECT id,name FROM table_name;
   ```
 * INSERT   
   插入语句要求没有明确给出的列必须有自己的默认值。但由于可能会更新索引，其他命令会被延迟执行。不过也有相应指定优先级的方法（LOW_PRIORITY）。
   ```sql
   # 下面的插入方法并不安全，因为它高度依赖表中列的顺序！
   INSERT INTO table_name(
     NULL,
     'moon',
     'huanggang'
     );
   ```
   ```sql
   # 更安全的方法，先指定好添加各项数据对应的列名
   INSERT INTO table_name(
     id,name,hometown
   )
   VALUES(
     NULL,'carpenter','huanggang'
   );
   ```
   ```sql
   # 一次插入多行
   INSERT INTO table_name(
     id,name,hometown
   )
   VALUES(
     NULL,'daydream','brain'
   ),(
     NULL,'daydreams','our mind'
   )
   ```
  * 将SELECT的结果插入表中

## 七. 特性
* 内部引擎
* 自动增量
* 存储过程
* 游标
* 触发器
