static int cmpstrp(const void *p1, const void *p2, void *arg)
{
	const int h = *(int*)(arg);
	STAT_INC_ITERS;
	return strcmp(*(char* const *)p1 + h, *(char* const *)p2 + h);
}

static const char** qsort_ref(const char** RESTRICT S, const char** RESTRICT T, size_t n, int h) {
	STAT_INC_CALLS;
	qsort_r(S, n, sizeof(char*), cmpstrp, &h);
	return S;
}
