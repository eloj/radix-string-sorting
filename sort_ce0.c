#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <ctype.h>
#include <sys/mman.h>

#ifndef RESTRICT
#define RESTRICT __restrict__
#endif

#if 0
static const char** radix_sort_CE0(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	size_t c[256] = { 0 };
	size_t b[256];

	// Generate histogram
	for (size_t i = 0 ; i < n ; ++i) {
		++c[(uint8_t)(S[i][h])];
	}

	// Generate prefix sums
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
			radix_sort_CE0(S+x, T+x, c[i], h+1);
		}
		x += c[i];
	}

	return S;
}
#endif

static size_t calls = 0;
static size_t iters = 0;
static size_t wasted_iters = 0;
static size_t bucket_use[256] = { 0 };

static const char** radix_sort_CE0_CB_BM1(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	uint64_t buse[4] = { 0 };
	uint64_t cuse[4] = { 0 }; // TODO: Could overwrite buse during prefix sum iteration (after k loop)
	uint8_t bones[256] = { 0 };
	size_t cb[256] = { 0 };

	++calls;

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
			++bucket_use[i];
			++iters;

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

static int map_file(const char *filename, void **data, size_t *size) {

	printf("mmap'ing '%s'\n", filename);
	FILE *f = fopen(filename, "rb");
	if (!f) {
		return 1;
	}

	int res = fseek(f, 0, SEEK_END);
	if (res != 0) {
		errx(1, "System has broken fseek() -- guess we should have used fstat instead, huh.");
	}
	size_t len = ftell(f) + 1; // add one for extra terminator.
	fseek(f, 0, SEEK_SET);

	void *base = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(f), 0);
	if (base == MAP_FAILED) {
		fclose(f);
		return 2;
	}
	fclose(f);

	*data = base;
	*size = len;

	return 0;
}

static size_t generate_string_array(char *data, size_t len, const char ***arr) {
	size_t entries = 0;

	char *p = data;
	while (p < data + len) {
		if (*p++ == '\n') {
			++entries;
		}
	}

	if (entries > 0) {
		*arr = malloc(entries * sizeof(const char*));

		p = data;
		char *b = p;
		entries = 0;
		while (p < data + len) {
			if (*p == '\n') {
				(*arr)[entries++] = b;
				*p = '\0';
				b = p + 1;
			}
			++p;
		}
	}

	return entries;
}


int main(int argc, char *argv[]) {
	const char *filename = argc > 1 ? argv[1] : "../test-shuffled.txt";

	void *input = NULL;
	size_t len = 0;
	if (map_file(filename, &input, &len) != 0) {
		fprintf(stderr, "Error mapping input file.\n");
		exit(1);
	}

	// madvise(input, len, MADV_HUGEPAGE);
	printf("Input at %p, size=%zu\n", input, len);

	const char** src = NULL;
	size_t entries = generate_string_array(input, len, &src);

	printf("%zu entries to sort.\n", entries);

	if (entries == 0) {
		printf("Nothing to do.\n");
		return 0;
	}

	const char** aux = malloc(entries * sizeof(const char*));

	const char **res = radix_sort_CE0_CB_BM1(src, aux, entries, 0);

	int used = 0;
	for (size_t i = 0 ; i < 256 ; ++i) {
		if (bucket_use[i] > 0) {
#if BUCKETUSE__
			printf("bucket_use[%zu]=%zu\n", i, bucket_use[i]);
#endif
			++used;
		}
	}
	printf("Buckets used %d, unused %d. %zu calls, %zu iterations (%zu wasted)\n", used, 256-used, calls, iters, wasted_iters);

#if VERIFY
	for (size_t i = 0 ; i < entries ; ++i) {
		if (i < 10)
			printf("%s\n", res[i]);
	}

	int ok = 1;
	for (size_t i=1 ; i < entries ; ++i) {
		if (strcmp(res[i-1], res[i]) > 0) {
			printf("ERROR: res[%zu]='%s' > res[%zu]='%s'\n", i-1, res[i-1], i, res[i]);
			ok = 0;
			break;
		}
	}
	if (ok) {
		printf("Sort verified.\n");
	} else {
		printf("Sort FAILED.\n");
	}
#endif

	free(aux);
	free(src);
	munmap(input, len);

	return 0;
}
