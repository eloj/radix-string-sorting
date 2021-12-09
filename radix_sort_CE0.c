/*
	Sequential MSD Radix Sort "CE0"
	Algorithm 2.2 in [TB18], adapted from [KR08; Ran07].
*/
static const char** radix_sort_CE0(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	size_t c[256] = { 0 };
	size_t b[256];

	STAT_INC_CALLS;

	// Generate histogram
	for (size_t i = 0 ; i < n ; ++i) {
		++c[(uint8_t)(S[i][h])];
	}

	// Generate exclusive scan (prefix sum)
	b[0] = 0;
	for (size_t i = 1 ; i < 256 ; ++i) {
		b[i] = b[i-1] + c[i-1];
	}

	// Sort
	for (size_t i = 0 ; i < n ; ++i) {
		uint8_t idx = S[i][h];
		T[b[idx]++] = S[i];
	}
	memcpy(S, T, n * sizeof(*S));

	// Recursively sort buckets, skipping the first which contains strings already in order.
	size_t x = c[0];
	for (size_t i = 1 ; i < 256 ; ++i) {
		if (c[i] > 1) {
			STAT_INC_BUCKET(i);
			radix_sort_CE0(S+x, T+x, c[i], h+1);
		} else {
			STAT_INC_WASTED_ITERS;
		}
		STAT_INC_ITERS;
		x += c[i];
	}

	return S;
}
