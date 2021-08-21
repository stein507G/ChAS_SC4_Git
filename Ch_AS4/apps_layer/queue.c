#include "config.h"
#include "queue.h"

#ifdef USED_RECV_QUEUE		//(+)210715 CJKIM, Received data process changing


#define		MAX_QUEUE		1024

typedef struct
{
	int			Head;
	int			Tail;
	char*		pBuffer;
} __attribute__((packed)) QUEUE, *PQUEUE;

//-------------------------------------------------------

static QUEUE g_tQueue;

int init_queue()
{
	int ret = 0;

	g_tQueue.Head = 0;
	g_tQueue.Tail = 0;
	g_tQueue.pBuffer = (char *)malloc(MAX_QUEUE);
	memset(g_tQueue.pBuffer, 0, MAX_QUEUE);
	
	return ret;
}

int deinit_queue()
{
	int ret = 0;

	free(g_tQueue.pBuffer);

	return ret;
}

void clear_queue()
{
	g_tQueue.Head = 0;
	g_tQueue.Tail = 0;

	memset(g_tQueue.pBuffer, 0, MAX_QUEUE);
}


int get_buffer_size()
{
	int ret = 0;

	ret = MAX_QUEUE;

	return ret;
}

int get_data_size(int pos)
{
	int ret = 0;

	MSS_MAC_disable_intr();
	ret = (pos <= g_tQueue.Tail) ? g_tQueue.Tail - pos : get_buffer_size() + g_tQueue.Tail - pos;
	MSS_MAC_enable_intr();

	return ret;
}

int get_free_size()
{
	int ret = 0;

	MSS_MAC_disable_intr();
	ret = get_buffer_size() - get_available();
	MSS_MAC_enable_intr();

	return ret;
}

int get_available()
{
	int ret = 0;

	MSS_MAC_disable_intr();
	ret = (g_tQueue.Head <= g_tQueue.Tail )? g_tQueue.Tail - g_tQueue.Head : get_buffer_size() + g_tQueue.Tail - g_tQueue.Head;
	MSS_MAC_enable_intr();

	return ret;
}

int get_head()
{
	int ret = 0;

	MSS_MAC_disable_intr();
	ret = g_tQueue.Head;
	MSS_MAC_enable_intr();

	return ret;
}

void set_head(int head)
{
	MSS_MAC_disable_intr();

	g_tQueue.Head = head % get_buffer_size();

	MSS_MAC_enable_intr();

}

int get_tail()
{
	int ret = 0;

	MSS_MAC_disable_intr();
	ret = g_tQueue.Tail;
	MSS_MAC_enable_intr();

	return ret;
}
int enqueue(const char* pBuffer, int offset, int size)
{
	int ret = 0;

	MSS_MAC_disable_intr();

	if (g_tQueue.Tail + size >= get_buffer_size()) {
		int copySize = get_buffer_size() - g_tQueue.Tail;
		memcpy(&g_tQueue.pBuffer[g_tQueue.Tail], &pBuffer[offset], copySize);
		memcpy(g_tQueue.pBuffer, &pBuffer[offset + copySize], size - copySize);
	}
	else {

		memcpy(&g_tQueue.pBuffer[g_tQueue.Tail], &pBuffer[offset], size);
	}
			
	g_tQueue.Tail = (g_tQueue.Tail + size) % get_buffer_size();

	ret = g_tQueue.Head <= g_tQueue.Tail ? g_tQueue.Tail - g_tQueue.Head : get_buffer_size() + g_tQueue.Tail - g_tQueue.Head;

	MSS_MAC_enable_intr();

	return ret;
}

int dequeue(char* pBuffer, int offset, int size)
{
	int ret = 0;

	MSS_MAC_disable_intr();

	if (g_tQueue.Head == g_tQueue.Tail)
		return 0;

	if (g_tQueue.Head + size > get_buffer_size()) {
		int copy = get_buffer_size() - g_tQueue.Head;

		memcpy(&pBuffer[offset], (const void*)&g_tQueue.pBuffer[g_tQueue.Head], copy);
		memcpy(&pBuffer[offset + copy], (const void*)g_tQueue.pBuffer, size - copy);
	}
	else {
		memcpy(&pBuffer[offset], (const void*)&g_tQueue.pBuffer[g_tQueue.Head], size);
	}

	g_tQueue.Head = (g_tQueue.Head + size) % get_buffer_size();

	ret = size;

	MSS_MAC_enable_intr();

	return ret;
}

int peek(char* pBuffer, int offset, int size)
{
	int ret = 0;

	MSS_MAC_disable_intr();

	if (g_tQueue.Head == g_tQueue.Tail)
		return 0;

	if (g_tQueue.Head + size > get_buffer_size()) {
		int copy = get_buffer_size() - g_tQueue.Head;

		memcpy(&pBuffer[offset], (const void*)&g_tQueue.pBuffer[g_tQueue.Head], copy);
		memcpy(&pBuffer[offset + copy], (const void*)g_tQueue.pBuffer, size - copy);
	}
	else {
		memcpy(&pBuffer[offset], (const void*)&g_tQueue.pBuffer[g_tQueue.Head], size);
	}

	ret = size;

	MSS_MAC_enable_intr();

	return ret;
}

#endif	//#ifdef USED_RECV_QUEUE

