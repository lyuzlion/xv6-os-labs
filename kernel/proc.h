
// #include "spinlock.h"

// Saved registers for kernel context switches.
struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.
  struct context context;     // swtch() here to enter scheduler().
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];

struct trapframe {
  /*   0 */ uint64 kernel_satp;  
  /*   8 */ uint64 kernel_sp;    
  /*  16 */ uint64 kernel_trap;  
  /*  24 */ uint64 epc;  
  /*  32 */ uint64 kernel_hartid; 
  /*  40 */ uint64 ra;
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
};

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

#define VMA_SIZE 16

struct mmap_vma{
  int in_use;     
  uint64 sta_addr;
  uint64 sz;   
  int prot;
  struct file* file; 
  int flags;  
};

struct proc {

  struct spinlock lock;

  enum procstate state;     
  void *chan;          
  int killed;        
  int xstate;       
  int pid;   

  struct proc *parent;       

  uint64 kstack;         
  uint64 sz;             
  pagetable_t pagetable;      
  struct trapframe *trapframe; 
  struct context context;     
  struct file *ofile[NOFILE]; 
  struct inode *cwd;
  char name[16];

  struct mmap_vma mmap_vmas[VMA_SIZE];
};
