#ifndef _MSX_H_SYS
#define _MSX_H_SYS

#define MSEC_PER_TICK   16      // assume 60Hz
#define MAX_PROCS 		10
#define BIOS_INT_HOOK  	0xFD9F

struct sys_proc {
    void  (*func)();
};

extern void sys_reboot();
extern byte sys_get_key(byte line);
extern byte sys_get_stick(byte port);
extern void sys_memcpy(byte * dst, byte * src, uint size);
#define     sys_memset __builtin_memset

extern void sys_proc_register(void (*func));
extern void sys_irq_init();
extern void sys_sleep(unsigned int time_ms);

#endif