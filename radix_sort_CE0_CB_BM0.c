/*
	Sequential MSD Radix Sort "CE0_CB_BM0"

	Based on "CE0_CB", but using a bitmap to optimize recursive sorting loop iteration.

*/
static const char** radix_sort_CE0_CB_BM0(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	uint64_t buse[4] = { 0 };
	size_t cb[256] = { 0 };

	STAT_INC_CALLS;

	// Generate histogram/character counts
	for (size_t i = 0 ; i < n ; ++i) {
		uint8_t idx = S[i][h];
		++cb[idx];
		buse[idx >> 6] |= (1UL << (idx & 63));
	}

	// Generate exclusive scan (prefix sum)
	size_t x = 0;
	for (size_t i = 0 ; i < 256 ; ++i) {
		size_t a = cb[i];
		cb[i] = x;
		x += a;
	}

	// Sort
	for (size_t i = 0 ; i < n ; ++i) {
		uint8_t idx = S[i][h];
		assert((buse[idx >> 6] & (1UL << (idx & 63))) != 0);
		T[cb[idx]++] = S[i];
	}
	memcpy(S, T, n * sizeof(*S));

	// Recursively sort buckets with more than one suffixes left.
	x = cb[0]; // This should be the count of '\0' in input.
	buse[0] &= ~1UL;
	for (int k = 0 ; k < 4 ; ++k) {
		uint64_t bitset = buse[k];
		while (bitset != 0) {
			size_t i = __builtin_ctzl(bitset) + (k * 64);

			assert(i > 0);
			assert(cb[i] >= x);

			size_t ci = (cb[i] - x);
			if (ci > 1) {
				radix_sort_CE0_CB_BM0(S+x, T, ci, h+1);
			} else {
				STAT_INC_WASTED_ITERS;
				++x;
			}
			STAT_INC_BUCKET(ci, i);
			STAT_INC_ITERS;
			x = cb[i];

			bitset ^= bitset & -bitset;
		}
	}

	return S;
}
