//
// Copyright © 2024 Agora
// This file is part of TEN Framework, an open source project.
// Licensed under the Apache License, Version 2.0, with certain conditions.
// Refer to the "LICENSE" file in the root directory for more information.
//
#include "include_internal/ten_runtime/msg/audio_frame/field/timestamp.h"

#include "include_internal/ten_runtime/common/constant_str.h"
#include "include_internal/ten_runtime/msg/audio_frame/audio_frame.h"
#include "include_internal/ten_runtime/msg/loop_fields.h"
#include "include_internal/ten_runtime/msg/msg.h"
#include "ten_runtime/msg/audio_frame/audio_frame.h"
#include "ten_runtime/msg/msg.h"
#include "ten_utils/lib/json.h"
#include "ten_utils/macro/check.h"
#include "ten_utils/macro/mark.h"

bool ten_audio_frame_put_timestamp_to_json(ten_msg_t *self, ten_json_t *json,
                                           ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self) &&
                 ten_raw_msg_get_type(self) == TEN_MSG_TYPE_AUDIO_FRAME,
             "Should not happen.");

  ten_json_t *ten_json =
      ten_json_object_peek_object_forcibly(json, TEN_STR_UNDERLINE_TEN);
  TEN_ASSERT(ten_json, "Should not happen.");

  ten_json_object_set_new(
      ten_json, TEN_STR_TIMESTAMP,
      ten_json_create_integer(
          ten_raw_audio_frame_get_timestamp((ten_audio_frame_t *)self)));

  return true;
}

bool ten_audio_frame_get_timestamp_from_json(ten_msg_t *self, ten_json_t *json,
                                             TEN_UNUSED ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self) &&
                 ten_raw_msg_get_type(self) == TEN_MSG_TYPE_AUDIO_FRAME,
             "Should not happen.");
  TEN_ASSERT(json && ten_json_check_integrity(json), "Should not happen.");

  ten_json_t *ten_json =
      ten_json_object_peek_object(json, TEN_STR_UNDERLINE_TEN);
  if (!ten_json) {
    return true;
  }

  int64_t timestamp = ten_json_object_get_integer(ten_json, TEN_STR_TIMESTAMP);
  ten_raw_audio_frame_set_timestamp((ten_audio_frame_t *)self, timestamp);

  return true;
}

void ten_audio_frame_copy_timestamp(ten_msg_t *self, ten_msg_t *src,
                                    TEN_UNUSED ten_list_t *excluded_field_ids) {
  TEN_ASSERT(self && src && ten_raw_msg_check_integrity(src) &&
                 ten_raw_msg_get_type(src) == TEN_MSG_TYPE_AUDIO_FRAME,
             "Should not happen.");

  ten_raw_audio_frame_set_timestamp(
      (ten_audio_frame_t *)self,
      ten_raw_audio_frame_get_timestamp((ten_audio_frame_t *)src));
}

bool ten_audio_frame_process_timestamp(ten_msg_t *self,
                                       ten_raw_msg_process_one_field_func_t cb,
                                       void *user_data, ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self), "Should not happen.");

  ten_msg_field_process_data_t timestamp_field;
  ten_msg_field_process_data_init(&timestamp_field, TEN_STR_TIMESTAMP,
                                  &((ten_audio_frame_t *)self)->timestamp,
                                  false);

  bool rc = cb(self, &timestamp_field, user_data, err);

  return rc;
}
