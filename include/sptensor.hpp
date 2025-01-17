#ifndef SPTENSOR_HPP_
#define SPTENSOR_HPP_


#include "common.hpp"


struct sptensor_struct {
  // number of modes/dimensions in the tensor
  int nmodes;

  // array that stores the length of each mode 
  IType* dims;

  // number of non-zeros
  IType nnz;

  // a 'num_modes' x 'nnz' matrix containing the indices of the non-zeros
  // index[0][1...nnz] stores the 1st mode indices
  // index[nmodes - 1][1...nnz] stores the last mode indices
  IType** cidx;

  // stores the non-zeros
  FType* vals;
};
typedef struct sptensor_struct SparseTensor;

typedef enum FileFormat_
{
    TEXT_FORMAT = 0,
    BINARY_FORMAT = 1
} FileFormat;

void ExportSparseTensor(const char *file_path, FileFormat f, SparseTensor *X);

void ImportSparseTensor(const char *file_path, FileFormat f, SparseTensor **X_);

void DestroySparseTensor(SparseTensor *X);

void CreateSparseTensor(
  IType mode, 
  IType* dims, 
  IType nnz,
  IType* cidx, 
  FType* vals, 
  SparseTensor** X_
);

SparseTensor * AllocSparseTensor(const int nnz, const int nmodes);
void sptensor_write(
  SparseTensor const * const sptensor,
  char const * const fname);

void sptensor_write_file(
  SparseTensor const * const sptensor,
  FILE * fout);

void PrintTensorInfo(IType rank, int max_iters, SparseTensor* X);
void PrintSparseTensor(SparseTensor* X);

#endif // SPTENSOR_HPP_
