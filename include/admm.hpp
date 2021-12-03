#ifndef ADMM_HPP_
#define ADMM_HPP_

#include "common.hpp"
#include "matrix.hpp"
#include "kruskal_model.hpp"
#include "constraints.hpp"

typedef struct 
{
    int nmodes;
    Matrix * mttkrp_buf; // the output of the mttkrp operation
    Matrix * auxil; // aux matrix for AO-ADMM factorization
    Matrix * duals[MAX_NUM_MODES]; // dual matrices for AO-ADMM factorization
    Matrix * mat_init; // Store the initial primal variable from each ADMM iteration
} admm_ws;

FType admm(
    IType mode,
    KruskalModel * M,
    Matrix ** aTa,
    FType * column_weights,
    cpd_constraint * con,
    admm_ws * ws
);

admm_ws * admm_ws_init(int nmodes);

#endif