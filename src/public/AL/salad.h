/*
 * salad.h
 * Created: 2021-07-26, 21:51:57.
 * Copyright (C) 2021, Kirill GPRB. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef AL_SALAD_H
#define AL_SALAD_H 1

#if defined(_MSC_VER)
#define SALAD_APIENTRY __cdecl
#define SALAD_ALX_APIENTRY __cdecl
#define SALAD_ALX_API __declspec(dllimport)
#else
#define SALAD_APIENTRY
#define SALAD_ALX_APIENTRY
#define SALAD_ALX_API
#endif

#if defined(__cplusplus)
#define SALAD_EXTERN extern "C"
#else
#define SALAD_EXTERN extern
#endif

typedef float               salad_float32_t;
typedef double              salad_float64_t;
typedef signed char         salad_int8_t;
typedef signed short        salad_int16_t;
typedef unsigned char       salad_uint8_t;
typedef unsigned short      salad_uint16_t;
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__GNUC__)
#include <stdint.h>
typedef int32_t             salad_int32_t;
typedef uint32_t            salad_uint32_t;
#elif defined(__cplusplus) && __cplusplus >= 201103L
#include <cstdint>
typedef std::int32_t        salad_int32_t;
typedef std::uint32_t       salad_uint32_t;
#elif defined(_WIN32)
typedef __int32             salad_int32_t;
typedef unsigned __int32    salad_uint32_t;
#else
typedef int                 salad_int32_t;
typedef unsigned int        salad_uint32_t;
#endif

/**
 * A function pointer responsible for loading OpenAL
 * functions from a dynamically linked library.
 * @param procname  Function name.
 * @param arg       Optional argument.
 * @return          Function pointer or NULL.
 */
typedef void *(SALAD_APIENTRY *SALAD_loadfunc_t)(const char *procname, void *arg);

/**
 * Loads OpenAL functions using default paths to libraries.
 * @return          Non-zero on success.
 */
SALAD_EXTERN int saladLoadAL(void);

/**
 * Loads OpenAL functions using a custom load function.
 * @param loadfunc  Load function (GetProcAddress).
 * @param arg       Optional argument (pass NULL if unused).
 * @return          Non-zero on success.
 */
SALAD_EXTERN int saladLoadALFunc(SALAD_loadfunc_t loadfunc, void *arg);

#endif
