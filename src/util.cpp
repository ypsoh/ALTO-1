#include "util.hpp"

Permutation * perm_alloc(
    IType const * const dims, int const nmodes) {
    Permutation * perm = (Permutation *) malloc(sizeof(Permutation));

    for (int m = 0; m < MAX_NUM_MODES; ++m) {
        if (m < nmodes) {
            perm->perms[m] = (IType *) malloc(dims[m] * sizeof(IType));
            perm->iperms[m] = (IType *) malloc(dims[m] * sizeof(IType));
        } else {
            perm->perms[m] = NULL;
            perm->iperms[m] = NULL;
        }
    }
    return perm;
}


Matrix * init_mat(IType nrows, IType ncols) {
    Matrix * mat = (Matrix *) AlignedMalloc(sizeof(Matrix));
    mat->I = nrows;
    mat->J = ncols;
    mat->vals = (FType *) AlignedMalloc(sizeof(FType*) * nrows * ncols);
    mat->rowmajor = 1;

    return mat;
};

Matrix * rand_mat(IType nrows, IType ncols) {
    Matrix * mat = init_mat(nrows, ncols);
    for (int i = 0; i < mat->I; ++i) {
        for (int j = 0; j < mat->J; ++j) {
            mat->vals[j + (i * ncols)] = ((FType) rand() / (FType) RAND_MAX);
        }
    }
    return mat;
};

Matrix * zero_mat(IType nrows, IType ncols) {
    Matrix * mat = init_mat(nrows, ncols);
    for (int i = 0; i < mat->I; ++i) {
        for (int j = 0; j < mat->J; ++j) {
            mat->vals[j + (i * ncols)] = 0.0;
        }
    }
    return mat;
};

void free_mat(Matrix * mat) {
    if (mat == NULL) return;
    free(mat->vals);
    free(mat);
};

FType rand_val()
{
  FType v =  3.0 * ((FType) rand() / (FType) RAND_MAX);
  if(rand() % 2 == 0) {
    v *= -1;
  }
  return v;
}


void fill_rand(FType * vals, IType num_el) {
    for(IType i=0; i < num_el; ++i) {
        vals[i] = rand_val();
    } 
}