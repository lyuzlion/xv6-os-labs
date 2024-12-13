#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "e1000_dev.h"
#include "net.h"
// #include <stddef.h>

#define TX_RING_SIZE 16
static struct tx_desc tx_ring[TX_RING_SIZE] __attribute__((aligned(16)));
static struct mbuf *tx_mbufs[TX_RING_SIZE];

#define RX_RING_SIZE 16
static struct rx_desc rx_ring[RX_RING_SIZE] __attribute__((aligned(16)));
static struct mbuf *rx_mbufs[RX_RING_SIZE];

static volatile uint32 *regs;

struct spinlock e1000_lock;

void
e1000_init(uint32 *xregs)
{
  int i;

  initlock(&e1000_lock, "e1000");

  regs = xregs;

  regs[E1000_IMS] = 0; // disable interrupts
  regs[E1000_CTL] |= E1000_CTL_RST;
  regs[E1000_IMS] = 0; // redisable interrupts
  memset(tx_ring, 0, sizeof(tx_ring));
  for (i = 0; i < TX_RING_SIZE; i++) {
    tx_ring[i].status = E1000_TXD_STAT_DD;
    tx_mbufs[i] = 0;
  }

  regs[E1000_TDBAL] = (uint64) tx_ring;
  if(sizeof(tx_ring) % 128 != 0)
    panic("e1000");
  regs[E1000_TDLEN] = sizeof(tx_ring);
  regs[E1000_TDH] = regs[E1000_TDT] = 0;
  memset(rx_ring, 0, sizeof(rx_ring));
  for (i = 0; i < RX_RING_SIZE; i++) {
    rx_mbufs[i] = mbufalloc(0);
    if (!rx_mbufs[i])
      panic("e1000");
    rx_ring[i].addr = (uint64) rx_mbufs[i]->head;
  }

  regs[E1000_RDBAL] = (uint64) rx_ring;
  if(sizeof(rx_ring) % 128 != 0)
    panic("e1000");
  regs[E1000_RDH] = 0;
  regs[E1000_RDT] = RX_RING_SIZE - 1;
  regs[E1000_RDLEN] = sizeof(rx_ring);

  regs[E1000_RA] = 0x12005452;
  regs[E1000_RA+1] = 0x5634 | (1<<31);
  
  for (int i = 0; i < 4096/32; i++)
    regs[E1000_MTA + i] = 0;

  regs[E1000_TCTL] = E1000_TCTL_EN |
    E1000_TCTL_PSP |  
    (0x10 << E1000_TCTL_CT_SHIFT) |
    (0x40 << E1000_TCTL_COLD_SHIFT);
  regs[E1000_TIPG] = 10 | (8<<10) | (6<<20);

  regs[E1000_RCTL] = E1000_RCTL_EN | 
    E1000_RCTL_BAM |          
    E1000_RCTL_SZ_2048 |       
    E1000_RCTL_SECRC;            
  
  regs[E1000_RDTR] = 0;  
  regs[E1000_RADV] = 0; 
  regs[E1000_IMS] = (1 << 7); 
}

int e1000_transmit(struct mbuf *m) // mbuf 是一个用于存储packet的结构体，包含packet的长度、内容等信息
{
	acquire(&e1000_lock); // 确保在发送数据包时不会有其他线程或中断干扰
	uint32 tdt = regs[E1000_TDT]; // 获得队列尾
	struct tx_desc *desc = &tx_ring[tdt]; 

	if(!(desc->status & E1000_TXD_STAT_DD)){
		release(&e1000_lock);
		return -1;
	}

	if(tx_mbufs[tdt]){ // 检查当前传输描述符对应的 mbuf 是否已经存在，如果存在，说明之前的数据包还没有被发送出去。
		mbuffree(tx_mbufs[tdt]); // 等待缓冲区空
	};

	tx_mbufs[tdt] = m; //存到缓冲区里

	desc->addr = (uint64)m->head;
	desc->length = m->len;
	desc->cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS;

	regs[E1000_TDT] = (tdt + 1) % TX_RING_SIZE; // 更新网卡的 TDT 寄存器，指向下一个传输描述符，如果已经到达队列末尾，则回到队列开头。
	release(&e1000_lock);

	return 0;
}

static void e1000_recv(void) {
	uint32 rdt = regs[E1000_RDT];
	uint32 tail = (rdt + 1) % RX_RING_SIZE;
	struct rx_desc *desc = &rx_ring[tail];

	while(desc->status & E1000_RXD_STAT_DD){

		if(desc->length > MBUF_SIZE) {
			panic("e1000 len");
		}

		rx_mbufs[tail]->len = desc->length;

		net_rx(rx_mbufs[tail]);

		rx_mbufs[tail] = mbufalloc(0);
		if(!rx_mbufs[tail]){
			panic("e1000_recv mbufalloc failed");
		}

		desc->addr = (uint64)rx_mbufs[tail]->head;
		desc->status = 0;

		tail = (tail + 1)%RX_RING_SIZE;
		desc = &rx_ring[tail];
	}
	regs[E1000_RDT] = (tail - 1) % RX_RING_SIZE;
}

void
e1000_intr(void)
{
  regs[E1000_ICR] = 0xffffffff;

  e1000_recv();
}
