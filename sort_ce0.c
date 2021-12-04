#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct sortrec {
	uint8_t key;
	const char *name;
};

struct sortrec const source_arr[] = {
	{ 255, "1st 255" },
	{ 45, "1st 45" },
	{ 3, "3" },
	{ 45, "2nd 45" },
	{ 2, "2" },
	{ 45, "3rd 45" },
	{ 1, "1" },
	{ 255, "2nd 255" },
};

static const char** radix_sort_CE0(const char **S, size_t n, int h) {
	size_t c[256] = { 0 };
	size_t b[256];

	// printf("Sorting %zu entries, prefix len %d\n", n, h);
	// TODO: return if n < 2?

	// Generate histogram/character counts
	for (size_t i = 0 ; i < n ; ++i) {
		// printf(">'%s' <- %d from %s'\n", S[i] + h, h, S[i]);
		++c[(uint8_t)(S[i][h])];
	}

	// Generate prefix sums
	b[0] = 0;
	for (size_t i = 1 ; i < 256 ; ++i) {
		b[i] = b[i-1] + c[i-1];
	}

	const char **T = malloc(n * sizeof(const char*));
	for (size_t i = 0 ; i < n ; ++i) {
		uint8_t idx = S[i][h];
		T[b[idx]] = S[i];
		++b[idx];
	}
	memcpy(S, T, n * sizeof(const char*));
	free(T);

	size_t x = c[0];
	for (size_t i = 1 ; i < 256 ; ++i) {
		if (c[i] > 1) {
			// printf("recurse S+%zu, n=%zu, h=%d\n", x, c[i], h+1);
			radix_sort_CE0(S+x, c[i], h+1);
			x += c[i];
		}
	}

	return S;
}

int main(int argc, char *argv[]) {
	size_t entries = sizeof(source_arr)/sizeof(source_arr[0]);

	const char** src = malloc(entries * sizeof(const char*));

	for (size_t i=0 ; i < entries ; ++i) {
		src[i] = source_arr[i].name;
	}

	for (size_t i=0 ; i < entries ; ++i) {
		printf("src[%zu]='%s'\n", i, src[i]);
	}

	const char **res = radix_sort_CE0(src, entries, 0);

	for (size_t i=0 ; i < entries ; ++i) {
		printf("res[%zu]='%s'\n", i, res[i]);
	}

	free(src);

	return 0;
}
