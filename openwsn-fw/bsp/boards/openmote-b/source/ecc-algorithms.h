/*
 * Copyright (c) 2014, Institute for Pervasive Computing, ETH Zurich.
 * All rights reserved.
 *
 * Author: Andreas Dröscher <contiki@anticat.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-ecc
 * @{
 *
 * \defgroup cc2538-ecc-algo cc2538 ECC Algorithms
 *
 * This is a implementation of ECDH, ECDSA sign and ECDSA verify. It
 * uses ecc-driver to communicate with the PKA. It uses continuations
 * to free the main CPU / thread while the PKA is calculating.
 *
 * \note
 * Only one request can be processed at a time.
 * Maximal supported key length is 384bit (12 words).
 * @{
 *
 * \file
 * Header file for the cc2538 ECC Algorithms
 */
#ifndef ECC_PROCESS_H_
#define ECC_PROCESS_H_

#include <headers/hw_ints.h>
#include <headers/hw_memmap.h>
#include <headers/hw_pka.h>
#include <headers/hw_types.h>
#include "interrupt.h"
#include "pka.h"
#include "sys_ctrl.h"
#include "debug.h"
#include "pka.h"
#include "ecc_curveinfo.h"

typedef struct  {
  //Containers for the State

  //Input Variables
  uint32_t    a[12];            //Left Number
  uint32_t    b[12];            //Right Number
  uint8_t     size;             //Length of a and b

  //Output Variables
  uint8_t     result;           //Result Code
} ecc_compare_state_t;

/**
 * \brief Do a compare of two big numbers
 *
 * This function can be used for ECDH as well as
 * Calculating a Public Key for ECDSA
 */
uint8_t ecc_compare(uint32_t a[12],            //Left Number
                    uint32_t b[12],            //Right Number
                    uint8_t  size);

typedef struct {
  //Containers for the State
//  struct pt      pt;
//  struct process *process;

  //Input Variables
  tECCCurveInfo* curve_info; //Curve defining the CyclicGroup
  tECPt        point_in;         //Generator Point
  uint32_t     secret[12];       //Secret

  //Variables Holding intermediate data (initialized/used internally)
  uint32_t    rv;               //Address of Next Result in PKA SRAM

  //Output Variables
  uint8_t     result;           //Result Code
  tECPt       point_out;        //Generated Point
} ecc_multiply_state_t;

/**
 * \brief Do a Multiplication on a EC
 *
 * This function can be used for ECDH as well as
 * Calculating a Public Key for ECDSA
 */
uint8_t ecc_multiply(ecc_multiply_state_t *state);

typedef struct {
  //Containers for the State
//  struct pt      pt;
//  struct process *process;

  //Input Variables
  tECCCurveInfo* curve_info; //Curve defining the CyclicGroup
  uint32_t    secret[12];       //Secret Key
  uint32_t    k_e[12];          //Ephemeral Key
  uint32_t    hash[12];         //Hash to be signed

  //Variables Holding intermediate data (initialized/used internally)
  uint32_t    rv;               //Address of Next Result in PKA SRAM
  uint32_t    k_e_inv[12];      //Inverted ephemeral Key
  uint32_t    len;              //Length of intermediate Result

  //Output Variables
  uint8_t     result;           //Result Code
  tECPt       point_r;          //Signature R (x coordinate)
  uint32_t    signature_s[24];  //Signature S
} ecc_dsa_sign_state_t;

static ecc_dsa_sign_state_t state = {
.curve_info = &nist_p_256,
.secret     = { 0x94A949FA, 0x401455A1, 0xAD7294CA, 0x896A33BB,
                0x7A80E714, 0x4321435B, 0x51247A14, 0x41C1CB6B },
.k_e        = { 0x1D1E1F20, 0x191A1B1C, 0x15161718, 0x11121314,
                0x0D0E0F10, 0x090A0B0C, 0x05060708, 0x01020304 },
.hash       = { 0x65637572, 0x20612073, 0x68206F66, 0x20686173,
                0x69732061, 0x68697320, 0x6F2C2054, 0x48616C6C },
};

/**
 * \brief Sign a Hash
 *
 * This function has to be called several times until the
 * pt state is EXIT
 * If the result code is 0 (SUCCESS) the signature can be
 * read from point_r and signature_s
 */
uint8_t ecc_dsa_sign(ecc_dsa_sign_state_t *state);

typedef struct {
  //Containers for the State
//  struct pt      pt;
//  struct process *process;

  //Input Variables
  tECCCurveInfo* curve_info; //Curve defining the CyclicGroup
  uint32_t    signature_r[12];  //Signature R
  uint32_t    signature_s[12];  //Signature S
  uint32_t    hash[12];         //Hash to be signed
  tECPt       public;           //Signature R (x coordinate)

  //Variables Holding intermediate data (initialized/used internally)
  uint32_t    rv;               //Address of Next Result in PKA SRAM
  uint32_t    s_inv[12];        //Inverted ephemeral Key
  uint32_t    u1[24];           //Intermediate result
  uint32_t    u2[24];           //Intermediate result
  tECPt       p1;               //Intermediate result
  tECPt       p2;               //Intermediate result
  uint32_t    len;              //Length of intermediate Result

  //Output Variables
  uint8_t     result;           //Result Code
} ecc_dsa_verify_state_t;

/**
 * \brief Verify Signature
 *
 * This function has to be called several times until the
 * pt state is EXIT
 * If the result code is 0 (SUCCESS) the verification
 * was success full.
 * \note some error codes signal internal errors
 * and others signal falls signatures.
 */
uint8_t ecc_dsa_verify(ecc_dsa_verify_state_t *state);

#endif /* ECC_PROCESS_H_ */

/**
 * @}
 * @
**/
