#include"pch.h"

#ifdef MYBENCHMARK

int sqrt_int(int n)
{
	int sqrt = 0;
	int shift = 15;
	int sqrt2;
	while (shift >= 0)
	{
		sqrt2 = ((sqrt << 1) + (1 << shift)) << shift;
		if (sqrt2 <= n)
		{
			sqrt += (1 << shift);
			n -= sqrt2;
		}
			shift--;
	}
	return sqrt;
}

int main(int argc,char** argv)
{
	int processId,processNum;
	double epiTime;
	bool* notPrime;

	int primeNum = 0, globalNum = 0;

	int nowPrime = 2;
	int proc0Size;

	int L, R, length;
	int first;
	int index = 0;
	int n;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
	MPI_Comm_size(MPI_COMM_WORLD, &processNum);
	MPI_Barrier(MPI_COMM_WORLD);
	epiTime = -MPI_Wtime();
	
	n = atoi(argv[1]);

	proc0Size = (n - 1) / processNum;
	if (proc0Size < sqrt_int(n))
	{
		puts("����������");
		MPI_Finalize();
		exit(1);
	}

	L = processId * (n - 1) / processNum + 2;
	R = (processId + 1) * (n - 1) / processNum + 1;
	length = R - L + 1;

	notPrime = new bool[length];
	memset(notPrime, 0x00000000, length);

	do 
	{
		if (nowPrime * nowPrime > L)
		{
			first = nowPrime * nowPrime - L;
		}
		else
		{
			if (L % nowPrime == 0)
			{
				first = 0;
			}
			else 
			{
				first = nowPrime - (L % nowPrime);
			}
		}
		
		for (int i = first; i < length; i += nowPrime)
		{
			notPrime[i] = true;
		}

		if (!processId)
		{
			while (notPrime[++index]);
			nowPrime = index + 2;
		}
		if (processNum > 1)
		{
			MPI_Bcast(&nowPrime, 1, MPI_INT, 0, MPI_COMM_WORLD);
		}
	} 
	while (nowPrime * nowPrime <= n);
	
	//ͳ�Ƹ���
	for (int i = 0; i < length; i++)
	{
		if (!notPrime[i])
		{
			primeNum++;
		}
	}

	//��Լ
	if (processNum > 1)
	{
		MPI_Reduce(&primeNum, &globalNum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	}

	epiTime += MPI_Wtime();
	if (!processId)
	{
		printf("There are %d primes less than or equal to %d\n",globalNum, n);
		printf("SIEVE (%d) %10.6f\n", processNum, epiTime);
	}

	MPI_Finalize();
}

#endif