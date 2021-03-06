#ifndef MUF_EVENT_H
#define MUF_EVENT_H

struct mufevent {
	struct mufevent *next;
	char *event;
	struct inst data;
};


int muf_event_dequeue(dbref prog);
int muf_event_dequeue_pid(int pid);
struct frame* muf_event_pid_frame(int pid);
int muf_event_controls(dbref player, int pid);
void muf_event_register(dbref player, dbref prog, struct frame *fr);
int muf_event_count(struct frame* fr);
void muf_event_add(struct frame *fr, char *event, struct inst *val);
void muf_event_purge(struct frame *fr);
void muf_event_process();

#endif

