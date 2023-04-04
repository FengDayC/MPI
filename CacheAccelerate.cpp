#include"pch.h"
#ifdef CACHEACCELERATE
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
	bool* notPrime0;

	int primeNum = 0, globalNum = 0;

	int nowPrime = 0;

	int L, R, length;
	int first, first0;
	int n, halfn;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
	MPI_Comm_size(MPI_COMM_WORLD, &processNum);
	MPI_Barrier(MPI_COMM_WORLD);
	epiTime = -MPI_Wtime();

	n = atoi(argv[1]);

	//halfn is the number of odds needed to be check
	halfn = n & 1 ? n >> 1 : (n >> 1) - 1;

	int sqrtn = N2O(sqrt_int(n));
	halfn -= sqrtn;

	//start from sqrtn
	L = processId * halfn / processNum + sqrtn;
	R = (processId + 1) * halfn / processNum + sqrtn - 1;
	length = R - L + 1;

	//std::cout << "[" << O2N(L) << "," << O2N(R) << "]" << std::endl;

	notPrime = new bool[length];
	memset(notPrime, 0x00, length);
	notPrime0 = new bool[sqrtn];
	memset(notPrime0, 0x00, sqrtn);

	do
	{
		//std::cout << "prime is " << nowPrime << std::endl;
		int gap = O2N(nowPrime);
		//p*p>L
		int pp = ((nowPrime << 1) + 6) * nowPrime + 3;

		first0 = pp;
		for (int i = first0; i < sqrtn; i += gap)
		{
			notPrime0[i] = true;
		}

		if (pp > L)
		{
			first = pp - L;
		}
		else
		{
			int k = (L - nowPrime) % gap;
			first = k ? gap - k : 0;
		}

		for (int i = first; i < length; i += gap)
		{
			notPrime[i] = true;
		}

		while (notPrime0[++nowPrime]);
	} while (nowPrime < sqrtn);

	for (int i = 0; i < length; i++)
	{
		if (!notPrime[i])
		{
			primeNum++;
			//printf("Prime=%d\n", O2N(L+i));
		}
	}

	if (!processId)
	{
		for (int i = 0; i < sqrtn; i++)
		{
			if (!notPrime0[i])
			{
				primeNum++;
			}
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