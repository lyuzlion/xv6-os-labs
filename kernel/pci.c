//
// simple PCI-Express initialization, only
// works for qemu and its e1000 card.
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

void
pci_init() // 始化PCI设备
{
  uint64 e1000_regs = 0x40000000L; // e1000 设备的内存映射 I/O 地址（MMIO）

  uint32  *ecam = (uint32 *) 0x30000000L; // QEMU将PCI设备配置信息存储在这里
  
  for(int dev = 0; dev < 32; dev++){ // 循环遍历PCI总线上的所有可能的设备。
    int bus = 0;
    int func = 0;
    int offset = 0;
    uint32 off = (bus << 16) | (dev << 11) | (func << 8) | (offset);
    volatile uint32 *base = ecam + off;
    uint32 id = base[0];
    
    if(id == 0x100e8086){ // 检查设备ID是否为e1000网卡的ID
      base[1] = 7;
      __sync_synchronize();

      for(int i = 0; i < 6; i++){
        uint32 old = base[4+i];

        // writing all 1's to the BAR causes it to be
        // replaced with its size.
        base[4+i] = 0xffffffff;
        __sync_synchronize();

        base[4+i] = old;
      }

      // tell the e1000 to reveal its registers at
      // physical address 0x40000000.
      base[4+0] = e1000_regs;

      e1000_init((uint32*)e1000_regs);
    }
  }
}
