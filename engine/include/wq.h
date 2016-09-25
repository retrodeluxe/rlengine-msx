#ifndef _WQ_H_
#define _WQ_H_


#define INIT_WORK(WORK, FUNC, DATA)     (WORK).func = (FUNC);\
                                        (WORK).data = (DATA);

#define WQ_BUF_SIZE 20

struct work_struct {
    void (*func)();
    unsigned char data;
    uint alarm_secs;
    uint alarm_msec;
};

struct work_queue {
    byte head;
    byte tail;
    struct work_struct cq[WQ_BUF_SIZE];
};

extern void wq_start();
extern int queue_work(struct work_struct *work, char data);
extern int queue_delayed_work(struct work_struct *work, uint delay_secs, uint delay_msec, char data);
#endif