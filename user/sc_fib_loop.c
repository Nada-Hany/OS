
#include <inc/lib.h>


int64 fibonacci(int n, int64 *memo);

void
_main(void)
{
	int index=0;
	char buff1[256];
//	atomic_readline("Please enter Fibonacci index:", buff1);
//	index = strtol(buff1, NULL, 10);

	index = 1000;

	int64 *memo = malloc((index+1) * sizeof(int64));

	int64 res = fibonacci(index, memo) ;

	free(memo);

	atomic_cprintf("Fibonacci #%d = %lld\n",index, res);
	//To indicate that it's completed successfully
	inctst();
	return;
}


int64 fibonacci(int n, int64 *memo)
{
	for (int i = 0; i <= n; ++i)
	{
		if (i <= 1)
			memo[i] = 1;
		else
			memo[i] = memo[i-1] + memo[i-2] ;
	}
	return memo[n];
}



