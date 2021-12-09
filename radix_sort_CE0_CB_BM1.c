/*
	Sequential MSD Radix Sort "CE0_CB_BM1"

	Based on "CE0_CB_BM0", but using second bitmap to only loop over buckets with >1 suffix to sort.
*/
static const char** radix_sort_CE0_CB_BM1(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	uint64_t buse[4] = { 0 };
	uint64_t cuse[4] = { 0 }; // TODO: Could overwrite buse during prefix sum iteration (after k loop), need to benchmark.
	uint8_t bones[256] = { 0 };
	size_t cb[256] = { 0 };

	STAT_INC_CALLS;

	// Generate histogram/character counts
	for (size_t i = 0 ; i < n ; ++i) {
		uint8_t idx = S[i][h];
		++cb[idx];
		buse[idx >> 6] |= (1UL << (idx & 63));
	}

	size_t x = 0;
	uint8_t ones_skipped = 0;
	for (int k = 0 ; k < 4 ; ++k) {
		uint64_t bitset = buse[k];
		while (bitset != 0) {
			size_t i = __builtin_ctzl(bitset) + (k * 64);

			size_t a = cb[i];
			cb[i] = x;
			x += a;

			if (a > 1) {
				cuse[i >> 6] |= (1UL << (i & 63));
				bones[i] = ones_skipped;
				ones_skipped = 0;
			} else if (a == 1 && i > 0) {
				++ones_skipped;
			}

			bitset ^= (bitset & -bitset);
		}
	}

	// Sort
	for (size_t i = 0 ; i < n ; ++i) {
		uint8_t idx = S[i][h];
		// assert((buse[idx >> 6] & (1UL << (idx & 63))) != 0);
		T[cb[idx]++] = S[i];
	}
	memcpy(S, T, n * sizeof(*S));

	// Recursively sort buckets with more than one suffixes left.
	x = cb[0]; // This should be the count of '\0' in input.
	cuse[0] &= ~1UL;
	for (int k = 0 ; k < 4 ; ++k) {
		uint64_t bitset = cuse[k];
		while (bitset != 0) {
			size_t i = __builtin_ctzl(bitset) + (k * 64);

			assert(i > 0);
			assert(cb[i] >= x);
			STAT_INC_ITERS;
			STAT_INC_BUCKET(i);

			// The count needs to be adjusted by 1 for every one-suffix iteration we've skipped over.
			x += bones[i];
			size_t ci = (cb[i] - x);

			assert(ci > 1);
			radix_sort_CE0_CB_BM1(S+x, T, ci, h+1);
			x = cb[i];

			bitset ^= bitset & -bitset;
		}
	}

	return S;
}
