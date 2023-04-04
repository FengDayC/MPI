#include"pch.h"
#ifdef SHEDEVEN
#define O2N(i) ((i<<1)+3)
#define N2O(i) ((i+3)>>1)

inline int sqrt_int(int n)
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

int main(int argc, char** argv)
{
	int processId, processNum;
	double epiTime;
	bool* notPrime;

	int primeNum = 0, globalNum = 0;

	int nowPrime = 0;
	int proc0Size;

	int L, R, length;
	int first;
	int n,halfn;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
	MPI_Comm_size(MPI_COMM_WORLD, &processNum);
	MPI_Barrier(MPI_COMM_WORLD);
	epiTime = -MPI_Wtime();

	n = atoi(argv[1]);

	//halfn is the number of odds needed to be check
	halfn = n & 1 ? n / 2 : (n / 2) - 1;

	proc0Size = halfn / processNum;
	if (O2N(proc0Size) < sqrt_int(n))
	{
		puts("Too many process");
		MPI_Finalize();
		exit(1);
	}

	L = processId * halfn / processNum;
	R = (processId + 1) * halfn / processNum - 1;
	length = R - L + 1;

	notPrime = new bool[length];
	memset(notPrime, 0x00000000, length);

	do
	{
		int gap = O2N(nowPrime);
		//p*p>L
		int pp = (2 * nowPrime + 6) * nowPrime + 3;
		if ( pp > L)
		{
			first = pp - L;
		}
		else
		{
			int k = (L - nowPrime) % gap;
			first = k ? gap - k : 0;
		}
		//std::cout << "[" << O2N(L) << "," << O2N(R) << "] nowPrime= "<< nowPrime << " first:" << O2N(L+first) << std::endl;

		for (int i = first; i < length; i += gap)
		{
			notPrime[i] = true;
		}

		if (!processId)
		{
			while (notPrime[++nowPrime]);
		}
		if (processNum > 1)
		{
			MPI_Bcast(&nowPrime, 1, MPI_INT, 0, MPI_COMM_WORLD);
		}
	} while (O2N(nowPrime)* O2N(nowPrime) <= n);

	for (int i = 0; i < length; i++)
	{
		if (!notPrime[i])
		{
			primeNum++;
			//printf("Prime=%d\n", O2N(L+i));
		}
	}
	//printf("[%d,%d] cnt:%d\n", Odd2Num(L), Odd2Num(R), primeNum);

	if (processNum > 1)
	{
		MPI_Reduce(&primeNum, &globalNum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	}

	epiTime += MPI_Wtime();
	if (!processId)
	{
		//add "2"
		printf("There are %d primes less than or equal to %d\n", globalNum + 1, n);
		printf("SIEVE (%d) %10.6f\n", processNum, epiTime);
	}

	MPI_Finalize();
}

#endif