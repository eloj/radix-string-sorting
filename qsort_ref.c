static int cmpstrp(const void *p1, const void *p2)
{
	STAT_INC_ITERS;
	return strcmp(*(char* const *)p1, *(char* const *)p2);
}

static const char** qsort_ref(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	STAT_INC_CALLS;
	qsort(S, n, sizeof(char*), cmpstrp);
	return S;
}
