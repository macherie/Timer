#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#define PRELOAD_START_TIMEOUT_MS 1000  // 1 seconds
static void start_wait_timer(void);

typedef struct {
	int retry_counts;
	int timer_created;
	timer_t timer_id;
} test_preload;

static  test_preload preload_retry_cb;

static void wait_timeout(void)
{
	printf("wait_timeout start !!!!!!\n");
	start_wait_timer();
}

static void start_wait_timer(void)
{
	int status;
	struct sigevent se;
	struct itimerspec ts;
	unsigned int timeout_ms = PRELOAD_START_TIMEOUT_MS;

	se.sigev_notify = SIGEV_THREAD;
	se.sigev_value.sival_ptr = &preload_retry_cb.timer_id;
	se.sigev_notify_function = wait_timeout;
	se.sigev_notify_attributes = NULL;

	status = timer_create(CLOCK_MONOTONIC, &se, &preload_retry_cb.timer_id);

	if (status == 0)
		preload_retry_cb.timer_created = 1;
	
	if (preload_retry_cb.timer_created == 1) {
		ts.it_value.tv_sec = timeout_ms/1000;
		ts.it_value.tv_nsec = 1000000*(timeout_ms%1000);
		ts.it_interval.tv_sec = 0;
		ts.it_interval.tv_nsec = 0;
		status = timer_settime(preload_retry_cb.timer_id, 0, &ts, 0);
		if (status == -1)
			printf("failed \n");
	}

}

static void stop_wait_timer(void)
{
	if (preload_retry_cb.timer_created == 1) {
		timer_delete(preload_retry_cb.timer_id);
		preload_retry_cb.timer_created = 0;
	}
}

int main(int argc, const char *argv[])
{
	
	memset(&preload_retry_cb, 0, sizeof(preload_retry_cb));

	start_wait_timer();
	while(1);

	return 0;
}
