/*
	Sequential MSD Radix Sort "CE0_CB"
	Based on "CE0", but with merged 'c' and 'b' arrays.
*/
static const char** radix_sort_CE0_CB(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	size_t cb[256] = { 0 };

	STAT_INC_CALLS;

	// Generate histogram/character counts
	for (size_t i = 0 ; i < n ; ++i) {
		++cb[(uint8_t)(S[i][h])];
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
		T[cb[idx]] = S[i];
		++cb[idx];
	}
	memcpy(S, T, n * sizeof(*S));

	// Recursively sort buckets, skipping the first which contains strings already in order.
	x = cb[0];
	for (size_t i = 1 ; i < 256 ; ++i) {
		size_t ci = cb[i] - cb[i-1];
		if (ci > 1) {
			STAT_INC_BUCKET(i);
			radix_sort_CE0_CB(S+x, T, ci, h+1);
		} else {
			STAT_INC_WASTED_ITERS;
		}
		STAT_INC_ITERS;
		x += ci;
	}

	return S;
}
