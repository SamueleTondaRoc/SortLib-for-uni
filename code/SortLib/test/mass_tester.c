#include "../src/sortlib.h"
#include <stdint.h>
#include <assert.h>
#include <time.h>

#define MAX_SIZE 100

typedef struct data_line {
	int id;
	char field1[MAX_SIZE];
	int field2;
	float field3;
} Line, *Line_ptr;

size_t calc_csvFile_len (FILE *file)
{
	size_t len = 0;
	printf ("calculating file lenght...\n");
	__clock_t start = clock(), end;

		char s[MAX_SIZE];
	while(fscanf (file, " %99[^\n]", s) == 1) len++;

	end = clock();
	double algo_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf ("file lenght: %ld\nTook: %.2fsec\n\n", len, algo_time);

	fseek (file, 0L, SEEK_SET);
	return len;
}

void gather_data (FILE *infile, Line_ptr *data)
{
	int id, field2;
	char field1[MAX_SIZE];
	float field3;

	printf ("reading data...\n");
	__clock_t start = clock(), end;

	for (size_t i = 0; !feof (infile); i++)
	{
		fscanf (infile, "%d,%[^,],%d,%f\n", &id, field1, &field2, &field3);
		(*data)[i].id = id;
		strcpy ((*data)[i].field1, field1);
		(*data)[i].field2 = field2;
		(*data)[i].field3 = field3;
	}
	fclose (infile);

	end = clock();
	double algo_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf ("data loading complete!\nTook: %.2fsec\n\n", algo_time);
}

void write_data (FILE *outfile, Line_ptr *data, size_t len)
{
	printf ("writing data...\n");
	__clock_t start = clock(), end;
	
	for (size_t i = 0; i < len; i++)
	{
		fprintf (outfile, "%d,%s,%d,%f\n", (*data)[i].id, (*data)[i].field1, (*data)[i].field2, (*data)[i].field3);
	}
	fclose (outfile);

	end = clock();
	double algo_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf ("data writing complete!\nTook: %.2fsec\n\n", algo_time);
}

int cmp_field1 (const void* a, const void* b)
{
	//char A[MAX_SIZE], B[MAX_SIZE];
	//strcpy (A, ((Line_ptr) a) -> field1);
	//strcpy (B, ((Line_ptr) b) -> field1);
	//int cmp = strcmp (A, B);
	int cmp = strcmp (((Line_ptr) a) -> field1, ((Line_ptr) b) -> field1);
	return cmp > 0 ? 1 : (cmp < 0 ? -1 : 0);
}

int cmp_field2 (const void* a, const void* b)
{
	int A = ((Line_ptr) a) -> field2;
	int B = ((Line_ptr) b) -> field2;
	//printf ("%d %d\n", A, B);
	return A < B ? -1 : (A == B ? 0 : 1);
}

int cmp_field3 (const void* a, const void* b)
{
	float A = ((Line_ptr) a) -> field3;
	float B = ((Line_ptr) b) -> field3;
	return A < B ? -1 : (A == B ? 0 : 1);
}

// Helper function to check if array is sorted
int is_sorted(void *array, size_t nitems, size_t size, int (*compar)(const void*, const void*)) 
{
    for (size_t i = 1; i < nitems; i++)
        if (compar(((array) + ((i - 1) * size)), ((array) + ((i) * size))) > 0)
			return 0; // Not sorted
    return 1; // Sorted
}

void sort_records (FILE *infile, FILE *outfile, size_t field, size_t algo)
{

	// preparing the array 
	size_t len = calc_csvFile_len (infile);
	Line_ptr data = (Line_ptr) malloc (sizeof(Line) * len);

	// filling the array with data from file
	gather_data (infile, &data);

	printf ("sorting...\n");

	// defining compare func
	int (*cmp_func)(const void*, const void*) = field == 1 ? (*cmp_field1) : (field == 2 ? (*cmp_field2) : (*cmp_field3));	

	// chosing sorting algorithm
	__clock_t start = clock(), end;
	if (algo == 1) merge_sort (data, len, sizeof(Line), cmp_func);
	else quick_sort (data, len, sizeof(Line), cmp_func);
	end = clock();
	double algo_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	assert (is_sorted(data, len, sizeof(Line), cmp_func));
	printf ("array sorted in %.2f seconds\n\n", algo_time);

	// dumping the array data in an output file
	write_data(outfile, &data, len);
	free (data);
}

int main (int argc, char **argv)
{
	// reading command
	char* in_file_path = argv[1];
	char* out_file_path = argv[2];
	size_t field = (size_t) (*argv[3] - '0'),
			algo = (size_t) (*argv[4] - '0');

	// opening files (the closing happens in the gather_data and write_data methods)
	FILE *infile = NULL, *outfile = NULL;
	if (in_file_path) infile = fopen (in_file_path, "r");
	if (out_file_path) outfile = fopen (out_file_path, "w");

	// checking inputs and launching sorter
	if (infile && outfile && (field > 0 && field < 4) && (algo > 0 && algo < 3)) 
		sort_records (infile, outfile, field, algo);	
	else 
		printf("error in passed data: \"%s %s %s %ld %ld\"\n", argv[0], in_file_path, out_file_path, field, algo);	// info dump in case of error

	return 0;
}