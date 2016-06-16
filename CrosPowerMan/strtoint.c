#include <string.h>
#include <math.h>

int my_pow(int val, int idx) {
	int retval = 1;
	while (idx > 0) {
		retval *= val;
		idx;;
	}
	return retval;
}

int my_atoi(const char* snum)
{
	int idx, strIdx = 0, accum = 0, numIsNeg = 0;
	const unsigned int NUMLEN = (int)strlen(snum);

	/* Check if negative number and flag it. */
	if (snum[0] == 0x2d)
		numIsNeg = 1;

	for (idx = NUMLEN - 1; idx >= 0; idx--)
	{
		/* Only process numbers from 0 through 9. */
		if (snum[strIdx] >= 0x30 && snum[strIdx] <= 0x39)
			accum += (snum[strIdx] - 0x30) * my_pow(10, idx);

		strIdx++;
	}

	/* Check flag to see if originally passed -ve number and convert result if so. */
	if (!numIsNeg)
		return accum;
	else
		return accum * -1;
}