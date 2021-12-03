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
    
    for (int i = 0; i < rank; ++i) {
        Phi->vals[i * rank + i] += rho;
    }

    mat_cholesky(Phi);

    /* for checking convergence */
    FType primal_norm     = 0.;
    FType dual_norm       = 0.;
    FType primal_residual = 0.;
    FType dual_residual   = 0.;
    
    Matrix * mat_init = ws->mat_init;
    // Matrix * mttkrp_buf = ws->mttkrp_buf;
    Matrix * auxil = ws->auxil;
    Matrix * duals[MAX_NUM_MODES];
    Matrix * primal;

    mat_hydrate(primal, M->U[mode], M->dims[mode], rank);

    for (int m = 0; m < nmodes; ++m) {
        duals[m] = ws->duals[m];
    }
    
    size_t bytes = sizeof(FType) * rank * M->dims[mode];
    for (int it = 0; it < 100; ++it) {
        // save starting point for convergence check
        // copy factor matrix for init_mat
        memcpy(mat_init->vals, primal->vals, bytes);
    }

/*
    // Compute number of chunks
    IType num_chunks = 1;
    IType const chunk_size = 50; // set statically
    if(chunk_size > 0) {
        num_chunks =  M->dims[mode] / chunk_size;
        if(M->dims[mode] % chunk_size > 0) {
            ++num_chunks;
        }
    }

    IType it = 0;
    #pragma omp parallel for schedule(dynamic) reduction(+:it) if(num_chunks > 1)
    for(IType c=0; c < num_chunks; ++c) {
        IType const start = c * chunk_size;
        IType const stop = (c == num_chunks-1) ? mats[mode]->I : (c+1)*chunk_size;
        IType const offset = start * rank;
        IType const nrows = stop - start;


        //admm workspace, just for dev, definitely refactor later
         

        //sub-matrix chunks 
        Matrix primal;
        Matrix auxil;
        Matrix dual;
        Matrix mttkrp;
        Matrix init_buf;

        // Auxil matrix for AO-ADMM factorization
        // Dual (factor) matrixes for AO-ADMM factorization
        // Store mttkrp results
        // Init primal variable for each ADMM iteration

        // extract all the workspaces

        mat_hydrate(&primal, mats[mode]->vals + offset, nrows, rank);
        mat_hydrate(&auxil, ws->auxil->vals + offset, nrows, rank);
        mat_hydrate(&dual, ws->duals[mode]->vals + offset, nrows, rank);
        mat_hydrate(&mttkrp, ws->mttkrp_buf->vals + offset, nrows, rank);
        mat_hydrate(&init_buf, ws->mat_init->vals + offset, nrows, rank);

        // should the ADMM kernels parallelize themselves?
        bool const should_parallelize = (num_chunks == 1);

        //Run ADMM until convergence and record total ADMM its per row
        IType const chunk_iters =  p_admm_iterate_chunk(&primal, &auxil, &dual,
            ws->gram, &mttkrp, &init_buf, mode, con, rho, ws, cpd_opts,
            global_opts, should_parallelize);
        it += chunk_iters * nrows;
    } // foreach chunk
*/
    free_mat(Phi);
    /* return average # iterations */
    // return (FType) it / (FType) mats[mode]->I;
    return 0.0;

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