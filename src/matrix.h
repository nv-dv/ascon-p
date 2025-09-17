#include <m4ri/m4ri.h>

// choose matrix framework
#define M4RI

#ifdef M4RI

typedef mzd_t* matrix;
typedef mzd_t* vector;

// Function declarations
matrix matrix_from_array(rci_t rows, rci_t cols, uint8_t* array);
void array_from_matrix(matrix mat, uint8_t* out);
matrix CCF(uint8_t* seed, int n);
matrix ICCF(uint8_t* seed, int n);

#endif
