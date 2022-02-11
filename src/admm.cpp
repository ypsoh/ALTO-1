#include "admm.hpp"


// static IType p_admm_iterate_chunk(
//     Matrix * primal,
//     Matrix * auxil,
//     Matrix * dual,
//     Matrix * cholesky,
//     Matrix * mttkrp_buf,
//     Matrix * init_buf,
//     IType mode,
//     cpd_constraint * const con,
//     FType const rho
//     /*
//     cpd_ws * const ws,
//     splatt_cpd_opts const * const cpd_opts,
//     splatt_global_opts const * const global_opts,
//     bool const should_parallelize*/
//     )
// {
//   IType const rank = primal->J;

//   /* for checking convergence */
//   FType primal_norm     = 0.;
//   FType dual_norm       = 0.;
//   FType primal_residual = 0.;
//   FType dual_residual   = 0.;

//   /* foreach inner iteration */
//   IType it;
//   for(it=0; it < cpd_opts->max_inner_iterations; ++it) {
//     /* save starting point for convergence check */
//     size_t const bytes = primal->I * rank * sizeof(*primal->vals);
//     if(should_parallelize) {
//       par_memcpy(init_buf->vals, primal->vals, bytes);
//     } else {
//       memcpy(init_buf->vals, primal->vals, bytes);
//     }

//     /* auxiliary = MTTKRP + (rho .* (primal + dual)) */
//     p_setup_auxiliary(primal, mttkrp_buf, dual, rho, auxil,
//         should_parallelize);

//     /* Cholesky against auxiliary */
//     mat_solve_cholesky(ws->gram, auxil);

//     p_setup_proximity(primal, auxil, dual, should_parallelize);

//     /* APPLY CONSTRAINT / REGULARIZATION */
//     if(con->prox_func != NULL) {
//       con->prox_func(primal->vals, primal->I, rank, 0, con->data, rho,
//           should_parallelize);
//     } else {
//       fprintf(stderr, "SPLATT: WARNING no proximity operator specified for "
//                       "constraint '%s'\n.", con->description);
//     }

//     /* update dual: U += (primal - auxiliary) */
//     dual_norm = p_update_dual(primal, auxil, dual, should_parallelize);

//     /* check ADMM convergence */
//     p_calc_residual(primal, auxil, init_buf, &primal_norm, &primal_residual,
//         &dual_residual, should_parallelize);

//     /* converged? */
//     if((primal_residual <= cpd_opts->inner_tolerance * primal_norm) &&
//        (dual_residual   <= cpd_opts->inner_tolerance * dual_norm)) {
//       ++it;
//       break;
//     }
//   }

//   return it;
// }

// This function contains the original
// pseudo_inverse_stream implementation
// However, it also adds constraint functionality
FType admm(
    IType mode,
    KruskalModel * M,
    Matrix ** aTa,
    FType * column_weights,
    cpd_constraint * con,
    admm_ws * ws
)
{   
    // Unpack variables
    FType ** fm = M->U;
    Matrix * mttkrp_buf = ws->mttkrp_buf;
    int nmodes = M->mode;
    IType rank = M->rank;

    // compute gram matrix
    Matrix * Phi = hadamard_product(aTa, nmodes, mode);

    if (con->solve_type == UNCONSTRAINED) {
        // Apply frobenius regularization (1e-12)
        // This helps stablity 
        for (int i = 0; i < Phi->I; ++i) {
            Phi->vals[i * Phi->I + i] += 1e-12;
        }
        pseudo_inverse(Phi, mttkrp_buf);
        // Copy back to factor matrix
        memcpy(M->U[mode], mttkrp_buf->vals, sizeof(FType) * rank * M->dims[mode]);
        return 0.0;
    }

    // Constrained version
    FType rho = mat_trace(Phi) / (FType) rank;
    
    // Debug
    // PrintMatrix("Phi matrix", Phi);
    for (int i = 0; i < rank; ++i) {
        Phi->vals[i * rank + i] += rho;
        // Phi->vals[i * rank + i] += 1e-12;
    }

    // Perform cholesky only once
    mat_cholesky(Phi);

    // Step 1. Set up matrix values used for admm
    FType * auxil = ws->auxil->vals;
    FType * mttkrp = mttkrp_buf->vals;
    FType * dual = ws->duals[mode]->vals;
    FType * mat_init = ws->mat_init->vals;

    // Matrix * factor_matrix = mat_fillptr(M->U[mode], M->dims[mode], rank);
    Matrix * factor_matrix = (Matrix *)malloc(sizeof(Matrix));
    mat_hydrate(factor_matrix, M->U[mode], M->dims[mode], rank);
    // Matrix * factor_matrix = mat_fillptr(M->U[mode], M->dims[mode], rank);

    FType * primal = factor_matrix->vals;

    int N = M->dims[mode] * rank;
    size_t bytes = N * sizeof(*primal);

    // Set up norm
    FType primal_norm = 0.0;
    FType dual_norm = 0.0;
    FType primal_residual = 0.0;
    FType dual_residual = 0.0;

    // ADMM subroutine
    IType it;
    for (it = 0; it < 100; ++it) {
        memcpy(mat_init, primal, bytes);
        // Setup auxiliary
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; ++i) {
            auxil[i] = mttkrp[i] + rho * (primal[i] + dual[i]);
        }

        // Cholesky solve
        mat_cholesky_solve(Phi, ws->auxil);

        // Setup proximity
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; ++i) {
            primal[i] = auxil[i] - dual[i];
        }

        // PrintMatrix("factor matrix after constraint", factor_matrix);
        // Apply Constraints and Regularization
        // PrintFPMatrix("primal before", primal, factor_matrix->I, factor_matrix->J);
        con->func(primal, factor_matrix->I, factor_matrix->J);
        // PrintFPMatrix("primal after", primal, factor_matrix->I, factor_matrix->J);
        // Update dual: U += (primal - auxil)
        FType dual_norm = 0.0;

        #pragma omp parallel for schedule(static) reduction(+:dual_norm)
        for (int i = 0; i < N; ++i) {
            dual[i] += primal[i] - auxil[i];
            dual_norm += dual[i] * dual[i];
        }
        
        int nrows = factor_matrix->I;
        int ncols = factor_matrix->J;

        primal_norm = 0.0;
        primal_residual = 0.0;
        dual_residual = 0.0;

        // Check ADMM convergence, calc residual
        // We need primal_norm, primal_residual, dual_residual
        #pragma omp parallel for reduction(+:primal_norm, primal_residual, dual_residual)
        for (int i = 0; i < nrows; ++i) {
            for (int j = 0; j < ncols; ++j) {
                int index = j + (i * ncols);
                FType p_diff = primal[index] - auxil[index];
                FType d_diff = primal[index] - mat_init[index];

                primal_norm += primal[index] * primal[index];
                primal_residual += p_diff * p_diff;
                dual_residual += d_diff * d_diff;
            }
        }

        // fprintf(stderr, "p_res, p_norm, d_res, d_nrom, %f, %f, %f, %f\n", primal_residual, primal_norm, dual_residual, dual_norm);
        // Converged ?
        if ((primal_residual <= 0.07 * primal_norm) && (dual_residual <= 0.07 * dual_norm)) {
            ++it;
            break;
        }
    }
    return it;
    // return 0.0;
}

admm_ws * admm_ws_init(int nmodes) {
    admm_ws * ws = (admm_ws *) malloc(sizeof(*ws));

    ws->nmodes = nmodes;
    ws->mttkrp_buf = NULL;
    ws->auxil = NULL;
    for (int m = 0; m < nmodes; ++m) {
        ws->duals[m] = NULL;
    }
    ws->mat_init = NULL;

    return ws;
}