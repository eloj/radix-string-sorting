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

// TODO: Eventually there will be very few of the 256 buckets used; eval bitmap iteration for -256- loops
static const char** radix_sort_CE0_CB(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	size_t cb[256] = { 0 };

	// Generate histogram/character counts
	for (size_t i = 0 ; i < n ; ++i) {
		++cb[(uint8_t)(S[i][h])];
	}

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

	// Recursively sort buckets with more than one suffixes left.
	x = cb[1]; // These are the zero-terminators, which we skip.
	for (size_t i = 1 ; i < 256 ; ++i) {
		size_t ci = cb[i] - cb[i-1];
		if (ci > 1) {
			// printf("recurse S+%zu, n=%zu, h=%d\n", x, ci, h+1);
			// We could call different sorts here, e.g for small n.
			radix_sort_CE0_CB(S+x, T, ci, h+1);
		}
		x += ci;
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
	printf("%zu strings sort.\n", entries);

	const char **res = radix_sort_CE0_CB(src, aux, entries, 0);

	for (size_t i = 0 ; i < 10 ; ++i) {
		printf("%s\n", res[i]);
	}

	int ok = 1;
	for (size_t i=1 ; i < entries ; ++i) {
		if (strcmp(res[i-1], res[i]) > 0) {
			ok = 0;
			break;
		}
	}
	if (ok) {
		printf("Sort verified.\n");
	} else {
		printf("Sort FAILED.\n");
	}

	free(aux);
	free(src);
	munmap(input, len);

	return 0;
}
