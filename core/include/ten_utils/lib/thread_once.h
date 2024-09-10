//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_utils/ten_config.h"

#if defined(_WIN32)
  #include <Windows.h>
  #define ten_thread_once_t INIT_ONCE
  #define TEN_THREAD_ONCE_INIT INIT_ONCE_STATIC_INIT
#else
  #include <pthread.h>
  #define ten_thread_once_t pthread_once_t
  #define TEN_THREAD_ONCE_INIT PTHREAD_ONCE_INIT
#endif

/**
 * @brief Initialize a thread-once object.
 * @param once Pointer to the thread-once object.
 * @param init_routine Pointer to the initialization routine.
 * @return 0 on success, or a negative error code.
 * @note This function is will guarantee that the initialization routine will be
 *       called only once.
 */
TEN_UTILS_API int ten_thread_once(ten_thread_once_t *once,
                                  void (*init_routine)(void));