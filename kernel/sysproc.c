#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

// M: we should optimize the gitpid() function
uint64
sys_getpid(void)
{
  return myproc()->pid;
  // return myproc()->usyscall_page->pid;
  // struct usyscall *u = (struct usyscall *)USYSCALL;
  // return u->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
// M: implementation of syscall sys_pagccess
// M: the function is used to check the accessed bits of the pages
// M: so we should clear the accessed bits of the pages after checking
int
sys_pgaccess(void)
{
  uint64 va;
  int pagenum;
  uint64 abitsaddr;

  argaddr(0, &va);// 获取起始虚拟地址
  argint(1, &pagenum);// 获取页的数量
  argaddr(2, &abitsaddr); // 获取存储访问位结果的地址

  uint64 maskbits = 0;
  struct proc *proc = myproc();

  for (int i = 0; i < pagenum; i++) {  // 遍历每个页面

    pte_t *pte = walk(proc->pagetable, va+i*PGSIZE, 0); // 使用 walk 函数查找每个页面的页表条目。

    if (pte == 0) // 如果页表条目不存在，触发错误并终止程序。
      panic("[ERROR] sys_pagccess page not exist.");

    if (PTE_FLAGS(*pte) & PTE_A) { // 检查访问位并设置掩码：
      maskbits = maskbits | (1L << i);
    }
    *pte = ((*pte&PTE_A) ^ *pte) ^ 0 ; // 清除页表条目的访问位 PTE_A，以便下次检查时能够重新检测。
  }

  if (copyout(proc->pagetable, abitsaddr, (char *)&maskbits, sizeof(maskbits)) < 0)
    panic("[ERROR] sys_pgacess copyout error"); // 使用 copyout 函数将 maskbits 复制到用户空间的 abitsaddr 地址。

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
