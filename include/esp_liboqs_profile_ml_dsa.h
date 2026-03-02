// SPDX-License-Identifier: MIT

#pragma once

/**
 * @file esp_liboqs_profile_ml_dsa.h
 * @brief Profiling instrumentation helpers for ML-DSA signature scheme
 * 
 * This header provides convenience macros and registration for profiling
 * ML-DSA (Dilithium) functions. Include this in sign.c to instrument functions.
 */

#ifdef CONFIG_LIBOQS_ENABLE_PROFILING

#include "esp_liboqs_profile.h"

// Profile indices for ML-DSA functions
enum {
    MLDSA_PROF_KEYPAIR = 0,
    MLDSA_PROF_SIGN_INTERNAL,
    MLDSA_PROF_SIGN,
    MLDSA_PROF_VERIFY_INTERNAL,
    MLDSA_PROF_VERIFY,
    
    // Key components
    MLDSA_PROF_UNPACK_SK,
    MLDSA_PROF_UNPACK_PK,
    MLDSA_PROF_PACK_SK,
    MLDSA_PROF_PACK_PK,
    MLDSA_PROF_PACK_SIG,
    MLDSA_PROF_UNPACK_SIG,
    
    // Matrix operations
    MLDSA_PROF_POLYVEC_MATRIX_EXPAND,
    MLDSA_PROF_POLYVEC_MATRIX_POINTWISE,
    
    // NTT operations
    MLDSA_PROF_POLYVECL_NTT,
    MLDSA_PROF_POLYVECK_NTT,
    MLDSA_PROF_POLYVECL_INVNTT,
    MLDSA_PROF_POLYVECK_INVNTT,
    MLDSA_PROF_POLY_NTT,
    MLDSA_PROF_POLY_INVNTT,
    
    // Sampling
    MLDSA_PROF_POLYVECL_UNIFORM_ETA,
    MLDSA_PROF_POLYVECK_UNIFORM_ETA,
    MLDSA_PROF_POLYVECL_UNIFORM_GAMMA1,
    MLDSA_PROF_POLY_CHALLENGE,
    
    // Arithmetic
    MLDSA_PROF_POLYVECL_POINTWISE_POLY,
    MLDSA_PROF_POLYVECK_POINTWISE_POLY,
    MLDSA_PROF_POLYVECK_ADD,
    MLDSA_PROF_POLYVECK_SUB,
    MLDSA_PROF_POLYVECK_REDUCE,
    MLDSA_PROF_POLYVECL_REDUCE,
    
    // Rounding and decomposition
    MLDSA_PROF_POLYVECK_POWER2ROUND,
    MLDSA_PROF_POLYVECK_DECOMPOSE,
    MLDSA_PROF_POLYVECK_MAKE_HINT,
    MLDSA_PROF_POLYVECK_USE_HINT,
    
    // Norm checks
    MLDSA_PROF_POLYVECL_CHKNORM,
    MLDSA_PROF_POLYVECK_CHKNORM,
    
    // Hash operations
    MLDSA_PROF_SHAKE256,
    MLDSA_PROF_SHAKE256_INC_INIT,
    MLDSA_PROF_SHAKE256_INC_ABSORB,
    MLDSA_PROF_SHAKE256_INC_FINALIZE,
    MLDSA_PROF_SHAKE256_INC_SQUEEZE,
    
    // Rejection loop
    MLDSA_PROF_REJECTION_LOOP,
    
    MLDSA_PROF_MAX
};

// Registration function - call once during initialization
static inline void esp_liboqs_profile_ml_dsa_init(int *profile_ids) {
    profile_ids[MLDSA_PROF_KEYPAIR] = esp_liboqs_profile_register("crypto_sign_keypair");
    profile_ids[MLDSA_PROF_SIGN_INTERNAL] = esp_liboqs_profile_register("crypto_sign_signature_internal");
    profile_ids[MLDSA_PROF_SIGN] = esp_liboqs_profile_register("crypto_sign_signature");
    profile_ids[MLDSA_PROF_VERIFY_INTERNAL] = esp_liboqs_profile_register("crypto_sign_verify_internal");
    profile_ids[MLDSA_PROF_VERIFY] = esp_liboqs_profile_register("crypto_sign_verify");
    
    profile_ids[MLDSA_PROF_UNPACK_SK] = esp_liboqs_profile_register("unpack_sk");
    profile_ids[MLDSA_PROF_UNPACK_PK] = esp_liboqs_profile_register("unpack_pk");
    profile_ids[MLDSA_PROF_PACK_SK] = esp_liboqs_profile_register("pack_sk");
    profile_ids[MLDSA_PROF_PACK_PK] = esp_liboqs_profile_register("pack_pk");
    profile_ids[MLDSA_PROF_PACK_SIG] = esp_liboqs_profile_register("pack_sig");
    profile_ids[MLDSA_PROF_UNPACK_SIG] = esp_liboqs_profile_register("unpack_sig");
    
    profile_ids[MLDSA_PROF_POLYVEC_MATRIX_EXPAND] = esp_liboqs_profile_register("polyvec_matrix_expand");
    profile_ids[MLDSA_PROF_POLYVEC_MATRIX_POINTWISE] = esp_liboqs_profile_register("polyvec_matrix_pointwise_montgomery");
    
    profile_ids[MLDSA_PROF_POLYVECL_NTT] = esp_liboqs_profile_register("polyvecl_ntt");
    profile_ids[MLDSA_PROF_POLYVECK_NTT] = esp_liboqs_profile_register("polyveck_ntt");
    profile_ids[MLDSA_PROF_POLYVECL_INVNTT] = esp_liboqs_profile_register("polyvecl_invntt_tomont");
    profile_ids[MLDSA_PROF_POLYVECK_INVNTT] = esp_liboqs_profile_register("polyveck_invntt_tomont");
    profile_ids[MLDSA_PROF_POLY_NTT] = esp_liboqs_profile_register("poly_ntt");
    profile_ids[MLDSA_PROF_POLY_INVNTT] = esp_liboqs_profile_register("poly_invntt_tomont");
    
    profile_ids[MLDSA_PROF_POLYVECL_UNIFORM_ETA] = esp_liboqs_profile_register("polyvecl_uniform_eta");
    profile_ids[MLDSA_PROF_POLYVECK_UNIFORM_ETA] = esp_liboqs_profile_register("polyveck_uniform_eta");
    profile_ids[MLDSA_PROF_POLYVECL_UNIFORM_GAMMA1] = esp_liboqs_profile_register("polyvecl_uniform_gamma1");
    profile_ids[MLDSA_PROF_POLY_CHALLENGE] = esp_liboqs_profile_register("poly_challenge");
    
    profile_ids[MLDSA_PROF_POLYVECL_POINTWISE_POLY] = esp_liboqs_profile_register("polyvecl_pointwise_poly_montgomery");
    profile_ids[MLDSA_PROF_POLYVECK_POINTWISE_POLY] = esp_liboqs_profile_register("polyveck_pointwise_poly_montgomery");
    profile_ids[MLDSA_PROF_POLYVECK_ADD] = esp_liboqs_profile_register("polyveck_add");
    profile_ids[MLDSA_PROF_POLYVECK_SUB] = esp_liboqs_profile_register("polyveck_sub");
    profile_ids[MLDSA_PROF_POLYVECK_REDUCE] = esp_liboqs_profile_register("polyveck_reduce");
    profile_ids[MLDSA_PROF_POLYVECL_REDUCE] = esp_liboqs_profile_register("polyvecl_reduce");
    
    profile_ids[MLDSA_PROF_POLYVECK_POWER2ROUND] = esp_liboqs_profile_register("polyveck_power2round");
    profile_ids[MLDSA_PROF_POLYVECK_DECOMPOSE] = esp_liboqs_profile_register("polyveck_decompose");
    profile_ids[MLDSA_PROF_POLYVECK_MAKE_HINT] = esp_liboqs_profile_register("polyveck_make_hint");
    profile_ids[MLDSA_PROF_POLYVECK_USE_HINT] = esp_liboqs_profile_register("polyveck_use_hint");
    
    profile_ids[MLDSA_PROF_POLYVECL_CHKNORM] = esp_liboqs_profile_register("polyvecl_chknorm");
    profile_ids[MLDSA_PROF_POLYVECK_CHKNORM] = esp_liboqs_profile_register("polyveck_chknorm");
    
    profile_ids[MLDSA_PROF_SHAKE256] = esp_liboqs_profile_register("shake256");
    profile_ids[MLDSA_PROF_SHAKE256_INC_INIT] = esp_liboqs_profile_register("shake256_inc_init");
    profile_ids[MLDSA_PROF_SHAKE256_INC_ABSORB] = esp_liboqs_profile_register("shake256_inc_absorb");
    profile_ids[MLDSA_PROF_SHAKE256_INC_FINALIZE] = esp_liboqs_profile_register("shake256_inc_finalize");
    profile_ids[MLDSA_PROF_SHAKE256_INC_SQUEEZE] = esp_liboqs_profile_register("shake256_inc_squeeze");
    
    profile_ids[MLDSA_PROF_REJECTION_LOOP] = esp_liboqs_profile_register("rejection_loop_iteration");
}

#else
// Profiling disabled - empty inline function
static inline void esp_liboqs_profile_ml_dsa_init(int *profile_ids) {
    (void)profile_ids;
}
#endif // CONFIG_LIBOQS_ENABLE_PROFILING
