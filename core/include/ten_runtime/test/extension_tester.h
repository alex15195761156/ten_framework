//
// Copyright © 2024 Agora
// This file is part of TEN Framework, an open source project.
// Licensed under the Apache License, Version 2.0, with certain conditions.
// Refer to the "LICENSE" file in the root directory for more information.
//
#include "ten_runtime/ten_config.h"

#include "ten_utils/lib/smart_ptr.h"

typedef struct ten_extension_tester_t ten_extension_tester_t;
typedef struct ten_env_tester_t ten_env_tester_t;

typedef void (*ten_extension_tester_on_start_func_t)(
    ten_extension_tester_t *self, ten_env_tester_t *ten_env);

typedef void (*ten_extension_tester_on_cmd_func_t)(ten_extension_tester_t *self,
                                                   ten_env_tester_t *ten_env,
                                                   ten_shared_ptr_t *cmd);

TEN_RUNTIME_API ten_extension_tester_t *ten_extension_tester_create(
    ten_extension_tester_on_start_func_t on_start,
    ten_extension_tester_on_cmd_func_t on_cmd);

TEN_RUNTIME_API void ten_extension_tester_destroy(ten_extension_tester_t *self);

TEN_RUNTIME_API void ten_extension_tester_add_addon(
    ten_extension_tester_t *self, const char *addon_name);

TEN_RUNTIME_API void ten_extension_tester_run(ten_extension_tester_t *self);

TEN_RUNTIME_API ten_env_tester_t *ten_extension_tester_get_ten_env_tester(
    ten_extension_tester_t *self);