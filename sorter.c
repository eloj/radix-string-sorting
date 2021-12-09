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

typedef const char** (*radix_sorter_fp)(const char** RESTRICT, const char** RESTRICT, size_t, int);

#if defined STATS
#define STAT_INC_CALLS ++calls
#define STAT_INC_ITERS ++iters
#define STAT_INC_WASTED_ITERS ++wasted_iters
#define STAT_INC_BUCKET(bucket) ++bucket_use[(bucket)]
#else
#define STAT_INC_CALLS
#define STAT_INC_ITERS
#define STAT_INC_WASTED_ITERS
#define STAT_INC_BUCKET(bucket)
#endif

static size_t calls = 0;
static size_t iters = 0;
static size_t wasted_iters = 0;
static size_t bucket_use[256] = { 0 };

#include "radix_sort_CE0.c"
#include "radix_sort_CE0_CB.c"
#include "radix_sort_CE0_CB_BM1.c"

struct radix_sorter_t {
	const char *name;
	radix_sorter_fp func;
} radix_sorters[] = {
	{
		.name = "MSD Radix Sort variant CE0",
		.func = radix_sort_CE0
	},
	{
		.name = "MSD Radix Sort variant CE0_CB",
		.func = radix_sort_CE0_CB
	},
	{
		.name = "MSD Radix Sort variant CE0_CB_BM1",
		.func = radix_sort_CE0_CB_BM1
	},
};

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

static size_t generate_string_ptrs(char *data, size_t len, const char ***arr) {
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
	const size_t NUM_VARIANTS = sizeof(radix_sorters)/sizeof(radix_sorters[0]);

	const char *filename = argc > 1 ? argv[1] : "../test-shuffled.txt";
	int variant = argc > 2 ? atoi(argv[2]) : (int)(NUM_VARIANTS-1);

	if (variant >= (int)NUM_VARIANTS) {
		printf("Invalid variant selected (0-%zu)\n", NUM_VARIANTS-1);
		return 1;
	}

	printf("Selected variant %d: %s\n", variant, radix_sorters[variant].name);

	radix_sorter_fp radix_sorter = radix_sorters[variant].func;

	void *input = NULL;
	size_t len = 0;
	if (map_file(filename, &input, &len) != 0) {
		fprintf(stderr, "Error mapping input file.\n");
		exit(1);
	}

	// madvise(input, len, MADV_HUGEPAGE);
	printf("Input at %p, size=%zu\n", input, len);

	const char** src = NULL;
	size_t entries = generate_string_ptrs(input, len, &src);

	printf("%zu entries to sort.\n", entries);

	if (entries == 0) {
		printf("Nothing to do.\n");
		return 0;
	}

	const char** aux = malloc(entries * sizeof(const char*));

	const char **res = radix_sorter(src, aux, entries, 0);

#if STATS
	int used = 0;
	for (size_t i = 0 ; i < 256 ; ++i) {
		if (bucket_use[i] > 0) {
#if BUCKETUSE
			printf("bucket_use[%zu]=%zu\n", i, bucket_use[i]);
#endif
			++used;
		}
	}
	printf("Buckets used %d, unused %d. %zu calls, %zu iterations (%zu wasted)\n", used, 256-used, calls, iters, wasted_iters);
#endif

#if VERIFY
	for (size_t i = 0 ; i < entries ; ++i) {
		if (i < 10)
			printf("%s\n", res[i]);
		else
			break;
	}

	// TODO: Verify pointer output is permuation of pointer input.
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
