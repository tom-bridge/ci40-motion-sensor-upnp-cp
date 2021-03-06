#include "timeout.h"
#include <pthread.h>

void timeout_reset(TIMEOUT_S* stimeout)
{
	stimeout->b_elapsed = 0;
	clock_gettime(CLOCK_MONOTONIC, &stimeout->start);
}

int compare(TIMEOUT_S* stimeout)
{
	int res = 0;
	
	if(stimeout->b_elapsed == 0)
	{
		clock_gettime(CLOCK_MONOTONIC, &stimeout->end);
		
		if( stimeout->end.tv_sec - stimeout->start.tv_sec >= stimeout->sec_timeout)
		{
			stimeout->b_elapsed = 1;
			res =  1;
			stimeout->elapsed_cb(stimeout);
		}
	}
	
	return res;
}

void check_func(void* timeout_s)
{
	while(1)
	{
		compare((TIMEOUT_S*)timeout_s);
		sleep(1);
	}
}

void timeout_init_and_run(TIMEOUT_S* pstimeout)
{
	pthread_t timeout_check_thread;
	
	pstimeout->b_elapsed = 0;
	
	pthread_create( &timeout_check_thread, NULL, ((void *)check_func), pstimeout);
}


