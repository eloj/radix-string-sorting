#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef RESTRICT
#define RESTRICT __restrict__
#endif

struct sortrec {
	uint8_t key;
	const char *name;
};

struct sortrec const source_arr[] = {
	{ 255, "z" },
	{ 45, "a" },
	{ 3, "zz" },
	{ 0, "" },
	{ 2, "abracadabrakusinvitamin" },
	{ 45, "klopper" },
	{ 255, "" },
	{ 13, "klöpper" },
	{ 1, "klopper!aksjdkasjdk" },
};

static const char** radix_sort_CE0(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
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
			radix_sort_CE0(S+x, T, ci, h+1);
		}
		x += ci;
	}

	return S;
}

int main(int argc, char *argv[]) {
	size_t entries = sizeof(source_arr)/sizeof(source_arr[0]);

	const char** src = malloc(entries * sizeof(const char*));
	const char** aux = malloc(entries * sizeof(const char*));

	for (size_t i=0 ; i < entries ; ++i) {
		src[i] = source_arr[i].name;
	}

	for (size_t i=0 ; i < entries ; ++i) {
		printf("src[%zu]='%s'\n", i, src[i]);
	}

	const char **res = radix_sort_CE0(src, aux, entries, 0);

	for (size_t i=0 ; i < entries ; ++i) {
		printf("res[%zu]='%s'\n", i, res[i]);
	}

	free(aux);
	free(src);

	return 0;
}
