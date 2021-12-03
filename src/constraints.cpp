#include "constraints.hpp"

void constraint_lasso(
    FType * primal,
    IType nrows,
    IType ncols)
{
    // Add code here
};

void constraint_nonneg(
    FType * primal, 
    IType nrows, 
    IType ncols)
{
    #pragma omp parallel for
    for (IType i = 0; i < nrows; ++i) {
        for (IType j = 0; j < ncols; ++i) {
            IType idx = j + (i * ncols);
            primal[idx] = (primal[idx] > 0.0) ? primal[idx] : 0.0;
        }
    }
};

cpd_constraint *  init_constraint() {
  cpd_constraint * con;
  con = (cpd_constraint *) malloc(sizeof(*con));
  
  con->func = NULL;
  con->solve_type = UNCONSTRAINED;
  
  sprintf(con->description, "UNCONSTRAINED");
  return con;
};

void free_constraint(cpd_constraint * con) {
  if (con == NULL) return;
  free(con);
};

void apply_nonneg_constraint(cpd_constraint * con) {
  memset(con->description, 0, 256);
  sprintf(con->description, "NON-NEGATIVE");
  con->solve_type = NON_NEGATIVE;
  con->func = constraint_nonneg;
};

void apply_lasso_constraint(cpd_constraint * con) {
  memset(con->description, 0, 256);
  sprintf(con->description, "LASSO (SPARSITY)");
  con->solve_type = LASSO;
  con->func = constraint_nonneg;
};