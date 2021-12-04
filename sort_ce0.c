#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define SWAP(a, b) do { __auto_type T = (a); a = b; b = T; } while (0)

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
	{ 1, "klopper!aksjdkasjdk" },
};

static const char** radix_sort_CE0(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	size_t c[256] = { 0 };
	size_t b[256] = { 0 };
	size_t y[256] = { 0 };

	// printf("Sorting %zu entries, prefix len %d -- S @ %p, T @ %p\n", n, h, S, T);
	// TODO: return if n < 2?

	// Generate histogram/character counts
	for (size_t i = 0 ; i < n ; ++i) {
		// printf(">'%s' <- %d from %s'\n", S[i] + h, h, S[i]);
		++c[(uint8_t)(S[i][h])];
		++y[(uint8_t)(S[i][h])];
	}

	// Generate prefix sums
	b[0] = 0;
	for (size_t i = 1 ; i < 256 ; ++i) {
		b[i] = b[i-1] + c[i-1];
	}
	size_t x = 0;
	for (size_t i = 0 ; i < 256 ; ++i) {
		size_t a = y[i];
		y[i] = x;
		x += a;
		printf("b[%zu]=%zu, y:=%zu\n", i, b[i], y[i]);
		assert(b[i] == y[i]);
	}

	// Sort
	for (size_t i = 0 ; i < n ; ++i) {
		uint8_t idx = S[i][h];
		T[b[idx]] = S[i];
		++b[idx];
	}
	memcpy(S, T, n * sizeof(const char*));

	assert(c[0] == b[1]);

	// Recursively sort buckets with more than one suffixes left.
	x = b[1]; // These are the zero-terminators, which we skip.
	for (size_t i = 1 ; i < 256 ; ++i) {
		size_t ci = b[i] - b[i-1];
		assert(c[i] == ci);
		if (ci > 1) {
			printf("recurse S+%zu, n=%zu, h=%d\n", x, ci, h+1);
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
