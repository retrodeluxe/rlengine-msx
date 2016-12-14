#ifndef _WQ_H_
#define _WQ_H_


#define INIT_WORK(WORK, FUNC)     (WORK).func = (FUNC);\
                                  (WORK).pending = 0;

#define WQ_BUF_SIZE 20

struct work_struct {
    void (*func)();
    unsigned char data;
    unsigned char pending;
    uint16_t alarm_secs;
    uint16_t alarm_msec;
};

struct work_queue {
    byte head;
    byte tail;
    struct work_struct *cq[WQ_BUF_SIZE];
};

extern void wq_start();
extern int queue_work(struct work_struct *work);
extern int queue_delayed_work(struct work_struct *work, uint16_t delay_secs, uint16_t delay_msec);
#endif