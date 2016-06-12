#include <time.h>

typedef int (*timeout_cb)(void* stimeout);

typedef struct
{
	struct 		timespec start;
	struct 		timespec end;
	int 		sec_timeout;
	int 		b_elapsed;
	timeout_cb 	elapsed_cb;	
	
} TIMEOUT_S;

void timeout_init_and_run(TIMEOUT_S* pstimeout);

void timeout_reset(TIMEOUT_S* stimeout);
