// 2010年 07月 28日 星期三 13:01:43 CST
// author: 李小丹(Li Shao Dan) 字 殊恒(shuheng)
// K.I.S.S
// S.P.O.T
// linux-2.6.XX/Document/
// linux-2.6.xx/kernel/futex.c
// http://blog.csdn.net/cybertan/article/details/8096863
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>


#include <sys/syscall.h>

#define  gettid( )    syscall(SYS_gettid)

#define futex(addr1, op, val, rel, addr2, val3)    \
    syscall(SYS_futex, addr1, op, val, rel, addr2, val3)

typedef struct futex_t {
    int wake;
    int lock;
    int wlock;
}futex_t;

inline static void futex_init(futex_t *);
inline static int futex_wake(futex_t *);
inline static int futex_wait(futex_t *);
inline static int futex_lock(futex_t *);
inline static int futex_unlock(futex_t *);


static void *work_thread(void *);
static void *work_send(void *);


int main()
{
	struct futex_t ftx;
	pthread_t tid[2];
	futex_init(&ftx);
	futex_wake(&ftx);




	pthread_create(&tid[0], 0, work_thread, (void *)&ftx);
	pthread_create(&tid[1], 0, work_thread, (void *)&ftx);

	pthread_join(tid[0], 0);
	pthread_join(tid[1], 0);
	return 0;
}

void *work_thread(void *p)
{
	struct futex_t *ftx = (struct futex_t *)p;
	//sleep(2);

	int count = 0;

	printf("%d\n", gettid( ));
	for(;;) {
		futex_wait(ftx);
		//futex_lock(ftx);

		printf("tid = %d, count = %ld\n", gettid( ), count);
		count++;
		usleep(1000000);

		//futex_unlock(ftx);
		futex_wake(ftx);
	}

	return (void *)0;
}


inline static void futex_init(futex_t *ftx)
{
	ftx->lock = 0;
	ftx->wake = 0;
	ftx->wlock = 0;
}

inline static int futex_wake(futex_t *ftx)
{
	__sync_fetch_and_add(&ftx->wake, 1);
	//__sync_lock_test_and_set(&ftx->wake, 1);
	return futex(&ftx->wake, FUTEX_WAKE, 1, 0, 0, 0);
}

inline static int futex_wait(futex_t *ftx)
{
	futex(&ftx->wlock, FUTEX_LOCK_PI, 0, 0, 0, 0);
	int ret = futex(&ftx->wake, FUTEX_WAIT, 0, 0, 0, 0);
	__sync_fetch_and_sub(&ftx->wake, 1);
	futex(&ftx->wlock, FUTEX_UNLOCK_PI, 0, 0, 0, 0);
	return (ret && errno == EWOULDBLOCK ? 1 : ret);
}

inline static int futex_lock(futex_t *ftx)
{
	return futex(&ftx->lock, FUTEX_LOCK_PI, 0, 0, 0, 0);
}

inline static int futex_unlock(futex_t *ftx)
{
	return futex(&ftx->lock, FUTEX_UNLOCK_PI, 0, 0, 0, 0);
}
