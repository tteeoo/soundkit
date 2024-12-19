typedef struct {
	double seed;
	double iter;
	double start;
	double end;
	int mapped;
	double *table;
	double (*fn) (double);
} sample;

typedef struct {
	sample s;
	double rate;
	double start;
	double inc;
	double end;
} container;
