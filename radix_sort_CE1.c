/*
	Sequential MSD Radix Sort "CE1" /w fissioned cache loop.
	Algorithm 2.3 in [TB18], adapted from [KR08; Ran07].
*/
static const char** radix_sort_CE1(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	size_t c[256] = { 0 };
	size_t b[256];

	STAT_INC_CALLS;
	unsigned char *o = malloc(n); // PERF: Could be passed on.

	// Cache input
	for (size_t i = 0 ; i < n ; ++i) {
		o[i] = S[i][h];
	}

	// Generate histogram (fissioned from loop above)
	for (size_t i = 0 ; i < n ; ++i) {
		++c[o[i]];
	}

	// Generate exclusive scan (prefix sum)
	b[0] = 0;
	for (size_t i = 1 ; i < 256 ; ++i) {
		b[i] = b[i-1] + c[i-1];
	}

	// Sort
	for (size_t i = 0 ; i < n ; ++i) {
		uint8_t key = o[i];
		T[b[key]] = S[i];
		++b[key];
	}
	memcpy(S, T, n * sizeof(*S));
	free(o);

	// Recursively sort buckets, skipping the first which contains strings already in order.
	size_t x = c[0];
	for (size_t i = 1 ; i < 256 ; ++i) {
		if (c[i] > 1) {
			radix_sort_CE1(S+x, T+x, c[i], h+1);
		} else {
			STAT_INC_WASTED_ITERS;
		}
		STAT_INC_BUCKET(c[i], i);
		STAT_INC_ITERS;
		x += c[i];
	}

	return S;
}
