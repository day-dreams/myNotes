python分布式任务队列Celery学习笔记
  
本笔记基于python2.7下的celery 4.0.2。

[Celery的api列表](http://docs.celeryproject.org/en/latest/reference/index.html)

<!-- TOC -->

- [1. 相关命令](#1-相关命令)
- [2. Application](#2-application)

<!-- /TOC -->

# 1. 相关命令

* 启动worker  
    celery -A proj worker -l info

* 停止当前的worker  
    Contorl-c

# 2. Application

```Application```是Celery的一个实例，celery的功能由这个Application提供。

这个application是线程安全的，所以可以同时运行多个不同配置、不同任务的application。

# 3. Tasks
> keywords:
>   task message
>   redelivered
>   idempotent
首先说说一些文档上的部分细节。

* 任务消息(task message)在整个分布式系统中传递，如果某个节点接受到了这个message，却由于某些原因没有成功完成（断电等）,这个message会被重新发送(redelivered)给其他节点去执行。

* 出于任务执行可能失败的考虑，我们需要确保task的执行符合幂等率(idempotent)，无论失败多少次，都不要造成为定义的行为。

## 3.1 Basics

* 用来制造task的装饰器keying可以带各种选项参数，包括最大尝试次数(max_reties)，延迟确认(acks_late)，期望异常(throws)

* 可以使用多层装饰器

* 可以对task进行绑定，以访问每次task的执行情况和其他基础信息:
    ```python
        logger = get_task_logger(__name__)

        @task(bind=True)
        def add(self, x, y):
            logger.info(self.request.id)
    ```


## 3.2 Task Name
celery是一个分布式的任务队列，它注册一系列的任务，供使用者(远程)调用。

那么使用者是如何调用的呢？首先排除传输代码来调用。实际上，调用这通过发送一个信息(message)，标记好需要调用的任务(task)，worker就开始进行相应的任务执行了。

Task Name就是每个task独一无二的标记。下面的代码使用装饰器，附加了用户自定义的操作。有两个被装饰好的task，分别完成不同的任务，他们的函数名add和mul就是各自的Task Name。
```python
@app.task
def add(x,y):
    return x+y

@app.task
def mul(x,y):
    return x*y

```

# 3. Routing and Queue

这里只讨论AMPQ作为broker时的情况.

在AMPQ中,有几个概念:Exchanges(交换机),queues(队列),message(消息)

当你调用task.your_task_name.delay()时,你的本地主机把这个调用信息通过AMBQ协议发送到broker主机,broker主机再根据这个task所属的队列,进行调用消息转发.接受到这个调用消息的worker(其启动时用-Q参数指定的队列必须和这个task所属队列相同)会执行这个task,同时将调用消息从队列中取走,那么其他worker服务的队列即使和这个task相同,也不再会执行这个task,因为调用消息已经被销毁了.所以,**当你有多个worker服务与同一个队列,一个task调用只会引发一个worker去工作**,这也是分布式消息队列的目标所在.如果你观察到多个worker都响应了消息,那么你一定看错了,否则请在这个笔记下面继续补充!
