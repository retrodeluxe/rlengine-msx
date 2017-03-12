#ifndef _MSX_H_SYS
#define _MSX_H_SYS

#define MSEC_PER_TICK   16      // assume 60Hz
#define MAX_PROCS 		10

#define BIOS_INT_HOOK 	0xFD9A

struct sys_proc {
    void  (*func)();
};

#define STICK_UP 			1
#define STICK_UP_RIGHT 		2
#define STICK_RIGHT 		3
#define STICK_DOWN_RIGHT  	4
#define STICK_DOWN 			5
#define STICK_DOWN_LEFT		6
#define STICK_LEFT 			7
#define STICK_UP_LEFT		8

extern void sys_reboot();
extern uint8_t sys_get_key(uint8_t line);
extern uint8_t sys_get_stick(uint8_t port);
extern void sys_memcpy(uint8_t * dst, uint8_t * src, uint16_t size);
#define     sys_memset __builtin_memset

extern void sys_proc_register(void (*func));
extern void sys_irq_init();
extern void sys_sleep(unsigned int time_ms);
extern uint16_t sys_gettime_secs();
extern uint16_t sys_gettime_msec();
extern uint16_t sys_get_ticks();
#endif
