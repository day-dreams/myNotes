数据竞争实例.
运行:
```bash
g++ -std=c++11 ./data-race.cpp -pthread;./a.out 
```
----------------------------------------

值得注意的是,调整每个线程的加操作次数,有可能得到正确结果5000.在我的机器上,每个线程只加1000次就可以的到正确结果.

所以推测:**每个线程的时间片大概在前次加法操作(包括load,add,store)的数量级上**.

这是mythread函数的汇编代码,可以看出count=count+1实际上是有3个指令完成的:load,add,store.

```as
  4006f6:       55                      push   %rbp
  4006f7:       48 89 e5                mov    %rsp,%rbp
  4006fa:       48 89 7d e8             mov    %rdi,-0x18(%rbp)
  4006fe:       c7 45 fc 00 00 00 00    movl   $0x0,-0x4(%rbp)
  400705:       81 7d fc e8 03 00 00    cmpl   $0x3e8,-0x4(%rbp)
  40070c:       74 15                   je     400723 <_Z8mythreadPv+0x2d>
  40070e:       8b 05 40 09 20 00       mov    0x200940(%rip),%eax        # 601054 <x>
  400714:       83 c0 01                add    $0x1,%eax
  400717:       89 05 37 09 20 00       mov    %eax,0x200937(%rip)        # 601054 <x>
  40071d:       83 45 fc 01             addl   $0x1,-0x4(%rbp)
  400721:       eb e2                   jmp    400705 <_Z8mythreadPv+0xf>
  400723:       90                      nop
  400724:       5d                      pop    %rbp
  400725:       c3                      retq   
```