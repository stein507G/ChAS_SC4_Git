#ifndef _CHROGEN_LC_QUEUE_H__
#define _CHROGEN_LC_QUEUE_H__

#ifdef USED_RECV_QUEUE		//(+)210715 CJKIM, Received data process changing

int init_queue();
int deinit_queue();
void clear_queue();

int get_buffer_size();
int get_data_size(int pos);
int get_free_size();
int get_available();

int get_head();
void set_head(int head);
int get_tail();

int enqueue(const char* pBuffer, int offset, int size);
int dequeue(char* pBuffer, int offset, int size);
int peek(char* pBuffer, int offset, int size);
#endif

#endif	//_CHROGEN_LC_QUEUE_H__
