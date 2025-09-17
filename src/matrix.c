#include "matrix.h"
#include <stdint.h>

// choose matrix framework
#define M4RI

# ifdef M4RI

typedef mzd_t* matrix;
typedef mzd_t* vector;

matrix matrix_from_array(rci_t rows, rci_t cols, uint8_t* array) {
    mzd_t *M = mzd_init(rows, cols);
    
    for (rci_t i = 0; i < rows; i++) {
        for (rci_t j = 0; j < cols; j++) {
            BIT bit = array[i * cols + j];  // row-major indexing
            mzd_write_bit(M, i, j, bit);
        }
    }
    return M;
}

void array_from_matrix(matrix mat, uint8_t* out) {
    for (rci_t i = 0; i < mat->nrows; i++) {
        for (rci_t j = 0; j < mat->ncols; j++) {
            BIT bit = mzd_read_bit(mat, i, j);
            out[i * mat->ncols + j] = bit;  // row-major indexing
        }
    }
}

matrix CCF(uint8_t* seed, int n) {

    matrix CCF_mat = mzd_init(n, n);
    mzd_write_bit(CCF_mat, 0, n-1, 1); 
    for (rci_t i = 1; i < n; i++)
    {
        mzd_write_bit(CCF_mat, i, i-1, 1); // ccf bit
        mzd_write_bit(CCF_mat, i, n-1, seed[i-1]); // seed bit 
    }
    return CCF_mat;

}

matrix ICCF(uint8_t* seed, int n) {
    
    matrix ICCF_mat = mzd_init(n, n);
    for (rci_t i = 0; i < n-1; i++)
    {
        mzd_write_bit(ICCF_mat, i, i+1, 1); // ccf bit
        mzd_write_bit(ICCF_mat, i, 0, seed[i]); // seed bit 
    }
    mzd_write_bit(ICCF_mat, n-1, 0, 1); 

    return ICCF_mat;
}

#endif