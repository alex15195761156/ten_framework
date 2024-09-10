//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "include_internal/ten_runtime/msg/video_frame/video_frame.h"

#include <stdlib.h>
#include <string.h>

#include "include_internal/ten_runtime/common/constant_str.h"
#include "include_internal/ten_runtime/msg/msg.h"
#include "include_internal/ten_runtime/msg/video_frame/field/field_info.h"
#include "include_internal/ten_utils/value/value_path.h"
#include "include_internal/ten_utils/value/value_set.h"
#include "ten_runtime/common/errno.h"
#include "ten_runtime/msg/msg.h"
#include "ten_runtime/msg/video_frame/video_frame.h"
#include "ten_utils/lib/alloc.h"
#include "ten_utils/lib/buf.h"
#include "ten_utils/lib/string.h"
#include "ten_utils/macro/check.h"
#include "ten_utils/value/value.h"
#include "ten_utils/value/value_get.h"

bool ten_raw_video_frame_check_integrity(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Should not happen.");

  if (ten_signature_get(&self->signature) !=
      (ten_signature_t)TEN_VIDEO_FRAME_SIGNATURE) {
    return false;
  }

  if (self->msg_hdr.type != TEN_MSG_TYPE_VIDEO_FRAME) {
    return false;
  }

  return true;
}

// Raw video frame
int64_t ten_raw_video_frame_get_timestamp(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_get_int64(&self->timestamp, NULL);
}

int32_t ten_raw_video_frame_get_width(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_get_int32(&self->width, NULL);
}

int32_t ten_raw_video_frame_get_height(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_get_int32(&self->height, NULL);
}

static ten_buf_t *ten_raw_video_frame_peek_data(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_peek_buf(&self->data);
}

ten_buf_t *ten_video_frame_peek_data(ten_shared_ptr_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_peek_data(ten_shared_ptr_get_data(self));
}

TEN_PIXEL_FMT ten_raw_video_frame_get_pixel_fmt(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_get_int32(&self->pixel_fmt, NULL);
}

bool ten_raw_video_frame_is_eof(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_get_bool(&self->is_eof, NULL);
}

bool ten_raw_video_frame_set_width(ten_video_frame_t *self, int32_t width) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_set_int32(&self->width, width);
}

bool ten_raw_video_frame_set_height(ten_video_frame_t *self, int32_t height) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_set_int32(&self->height, height);
}

bool ten_raw_video_frame_set_timestamp(ten_video_frame_t *self,
                                       int64_t timestamp) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_set_int64(&self->timestamp, timestamp);
}

bool ten_raw_video_frame_set_pixel_fmt(ten_video_frame_t *self,
                                       TEN_PIXEL_FMT pixel_fmt) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_set_int32(&self->pixel_fmt, pixel_fmt);
}

bool ten_raw_video_frame_set_is_eof(ten_video_frame_t *self, bool is_eof) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_value_set_bool(&self->is_eof, is_eof);
}

void ten_raw_video_frame_init(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Invalid argument.");

  ten_raw_msg_init(&self->msg_hdr, TEN_MSG_TYPE_VIDEO_FRAME);
  ten_signature_set(&self->signature, TEN_VIDEO_FRAME_SIGNATURE);

  ten_value_init_int32(&self->pixel_fmt, TEN_PIXEL_FMT_RGBA);
  ten_value_init_int64(&self->timestamp, 0);
  ten_value_init_int32(&self->width, 0);
  ten_value_init_int32(&self->height, 0);
  ten_value_init_bool(&self->is_eof, false);
  ten_value_init_buf(&self->data, 0);
}

static ten_video_frame_t *ten_raw_video_frame_create(void) {
  ten_video_frame_t *self = TEN_MALLOC(sizeof(ten_video_frame_t));
  TEN_ASSERT(self, "Failed to allocate memory.");

  ten_raw_video_frame_init(self);

  return self;
}

void ten_raw_video_frame_destroy(ten_video_frame_t *self) {
  TEN_ASSERT(self, "Should not happen.");

  ten_raw_msg_deinit(&self->msg_hdr);

  ten_value_deinit(&self->data);

  TEN_FREE(self);
}

ten_shared_ptr_t *ten_video_frame_create(void) {
  return ten_shared_ptr_create(ten_raw_video_frame_create(),
                               ten_raw_video_frame_destroy);
}

int32_t ten_video_frame_get_width(ten_shared_ptr_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_get_width(ten_shared_ptr_get_data(self));
}

bool ten_video_frame_set_width(ten_shared_ptr_t *self, int32_t width) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_set_width(ten_shared_ptr_get_data(self), width);
}

int32_t ten_video_frame_get_height(ten_shared_ptr_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_get_height(ten_shared_ptr_get_data(self));
}

bool ten_video_frame_set_height(ten_shared_ptr_t *self, int32_t height) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_set_height(ten_shared_ptr_get_data(self), height);
}

static uint8_t *ten_raw_video_frame_alloc_data(ten_video_frame_t *self,
                                               size_t size) {
  uint8_t *data = ten_value_peek_buf(&self->data)->data;
  if (data) {
    TEN_ASSERT(0, "Should not happen.");
    return NULL;
  }

  if (!ten_buf_init_with_owned_data(ten_value_peek_buf(&self->data), size)) {
    TEN_ASSERT(0, "Should not happen.");
    return NULL;
  }

  return ten_value_peek_buf(&self->data)->data;
}

uint8_t *ten_video_frame_alloc_data(ten_shared_ptr_t *self, size_t size) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_alloc_data(ten_shared_ptr_get_data(self), size);
}

int64_t ten_video_frame_get_timestamp(ten_shared_ptr_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_get_timestamp(ten_shared_ptr_get_data(self));
}

bool ten_video_frame_set_timestamp(ten_shared_ptr_t *self, int64_t timestamp) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_set_timestamp(ten_shared_ptr_get_data(self),
                                           timestamp);
}

TEN_PIXEL_FMT ten_video_frame_get_pixel_fmt(ten_shared_ptr_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_get_pixel_fmt(ten_shared_ptr_get_data(self));
}

bool ten_video_frame_set_pixel_fmt(ten_shared_ptr_t *self, TEN_PIXEL_FMT type) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_set_pixel_fmt(ten_shared_ptr_get_data(self), type);
}

bool ten_video_frame_is_eof(ten_shared_ptr_t *self) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_is_eof(ten_shared_ptr_get_data(self));
}

bool ten_video_frame_set_is_eof(ten_shared_ptr_t *self, bool is_eof) {
  TEN_ASSERT(self, "Should not happen.");
  return ten_raw_video_frame_set_is_eof(ten_shared_ptr_get_data(self), is_eof);
}

ten_msg_t *ten_raw_video_frame_as_msg_clone(ten_msg_t *self,
                                            ten_list_t *excluded_field_ids) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self) &&
                 ten_raw_msg_get_type(self) == TEN_MSG_TYPE_VIDEO_FRAME,
             "Should not happen.");

  ten_video_frame_t *new_frame =
      (ten_video_frame_t *)TEN_MALLOC(sizeof(ten_video_frame_t));
  TEN_ASSERT(new_frame, "Failed to allocate memory.");

  ten_raw_video_frame_init(new_frame);

  ten_video_frame_t *src_frame = (ten_video_frame_t *)self;
  ten_value_copy(&src_frame->timestamp, &new_frame->timestamp);
  ten_value_copy(&src_frame->width, &new_frame->width);
  ten_value_copy(&src_frame->height, &new_frame->height);
  ten_value_copy(&src_frame->is_eof, &new_frame->is_eof);
  ten_value_copy(&src_frame->pixel_fmt, &new_frame->pixel_fmt);
  ten_value_copy(&src_frame->data, &new_frame->data);

  for (size_t i = 0; i < ten_video_frame_fields_info_size; ++i) {
    if (excluded_field_ids) {
      bool skip = false;

      ten_list_foreach (excluded_field_ids, iter) {
        if (ten_video_frame_fields_info[i].field_id ==
            ten_int32_listnode_get(iter.node)) {
          skip = true;
          break;
        }
      }

      if (skip) {
        continue;
      }
    }

    ten_msg_copy_field_func_t copy_field =
        ten_video_frame_fields_info[i].copy_field;
    if (copy_field) {
      copy_field((ten_msg_t *)new_frame, self, excluded_field_ids);
    }
  }

  return (ten_msg_t *)new_frame;
}

ten_json_t *ten_raw_video_frame_as_msg_to_json(ten_msg_t *self,
                                               ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self) &&
                 ten_raw_msg_get_type(self) == TEN_MSG_TYPE_VIDEO_FRAME,
             "Should not happen.");

  ten_json_t *json = ten_json_create_object();
  TEN_ASSERT(json, "Should not happen.");

  if (!ten_raw_msg_put_field_to_json(self, json, err)) {
    ten_json_destroy(json);
    return NULL;
  }

  return json;
}

bool ten_raw_video_frame_check_type_and_name(ten_msg_t *self,
                                             const char *type_str,
                                             const char *name_str,
                                             ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self), "Invalid argument.");

  if (type_str) {
    if (strcmp(type_str, TEN_STR_VIDEO_FRAME) != 0) {
      if (err) {
        ten_error_set(err, TEN_ERRNO_GENERIC,
                      "Incorrect message type for video frame: %s", type_str);
      }
      return false;
    }
  }

  if (name_str) {
    if (strncmp(name_str, TEN_STR_MSG_NAME_TEN_NAMESPACE_PREFIX,
                strlen(TEN_STR_MSG_NAME_TEN_NAMESPACE_PREFIX)) == 0) {
      if (err) {
        ten_error_set(err, TEN_ERRNO_GENERIC,
                      "Incorrect message name for video frame: %s", name_str);
      }
      return false;
    }
  }

  return true;
}

static bool ten_raw_video_frame_init_from_json(ten_video_frame_t *self,
                                               ten_json_t *json,
                                               ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_video_frame_check_integrity(self),
             "Should not happen.");
  TEN_ASSERT(json && ten_json_check_integrity(json), "Should not happen.");

  for (size_t i = 0; i < ten_video_frame_fields_info_size; ++i) {
    ten_msg_get_field_from_json_func_t get_field_from_json =
        ten_video_frame_fields_info[i].get_field_from_json;
    if (get_field_from_json) {
      if (!get_field_from_json((ten_msg_t *)self, json, err)) {
        return false;
      }
    }
  }

  return true;
}

static ten_video_frame_t *ten_raw_video_frame_create_from_json(
    ten_json_t *json, ten_error_t *err) {
  TEN_ASSERT(json, "Should not happen.");

  ten_video_frame_t *video_frame = ten_raw_video_frame_create();
  TEN_ASSERT(video_frame && ten_raw_video_frame_check_integrity(
                                (ten_video_frame_t *)video_frame),
             "Should not happen.");

  if (!ten_raw_video_frame_init_from_json(video_frame, json, err)) {
    ten_raw_video_frame_destroy(video_frame);
    return NULL;
  }

  return video_frame;
}

static ten_video_frame_t *ten_raw_video_frame_create_from_json_string(
    const char *json_str, ten_error_t *err) {
  ten_json_t *json = ten_json_from_string(json_str, err);
  if (json == NULL) {
    return NULL;
  }

  ten_video_frame_t *video_frame =
      ten_raw_video_frame_create_from_json(json, err);

  ten_json_destroy(json);

  return video_frame;
}

ten_shared_ptr_t *ten_video_frame_create_from_json_string(const char *json_str,
                                                          ten_error_t *err) {
  ten_video_frame_t *video_frame =
      ten_raw_video_frame_create_from_json_string(json_str, err);
  return ten_shared_ptr_create(video_frame, ten_raw_video_frame_destroy);
}

ten_msg_t *ten_raw_video_frame_as_msg_create_from_json(ten_json_t *json,
                                                       ten_error_t *err) {
  TEN_ASSERT(json, "Should not happen.");

  return (ten_msg_t *)ten_raw_video_frame_create_from_json(json, err);
}

bool ten_raw_video_frame_as_msg_init_from_json(ten_msg_t *self,
                                               ten_json_t *json,
                                               ten_error_t *err) {
  TEN_ASSERT(
      self && ten_raw_video_frame_check_integrity((ten_video_frame_t *)self),
      "Should not happen.");
  TEN_ASSERT(json && ten_json_check_integrity(json), "Should not happen.");

  return ten_raw_video_frame_init_from_json((ten_video_frame_t *)self, json,
                                            err);
}

bool ten_raw_video_frame_set_ten_property(ten_msg_t *self, ten_list_t *paths,
                                          ten_value_t *value,
                                          ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self), "Invalid argument.");
  TEN_ASSERT(paths && ten_list_check_integrity(paths),
             "path should not be empty.");
  TEN_ASSERT(value && ten_value_check_integrity(value), "Invalid argument.");

  bool success = true;

  ten_error_t tmp_err;
  bool use_tmp_err = false;
  if (!err) {
    use_tmp_err = true;
    ten_error_init(&tmp_err);
    err = &tmp_err;
  }

  ten_video_frame_t *video_frame = (ten_video_frame_t *)self;

  ten_list_foreach (paths, item_iter) {
    ten_value_path_item_t *item = ten_ptr_listnode_get(item_iter.node);
    TEN_ASSERT(item, "Invalid argument.");

    switch (item->type) {
      case TEN_VALUE_PATH_ITEM_TYPE_OBJECT_ITEM: {
        if (!strcmp(TEN_STR_PIXEL_FMT,
                    ten_string_get_raw_str(&item->obj_item_str))) {
          const char *pixel_fmt_str = ten_value_peek_string(value);
          ten_raw_video_frame_set_pixel_fmt(
              video_frame,
              ten_video_frame_pixel_fmt_from_string(pixel_fmt_str));
          success = ten_error_is_success(err);
        } else if (!strcmp(TEN_STR_TIMESTAMP,
                           ten_string_get_raw_str(&item->obj_item_str))) {
          ten_raw_video_frame_set_timestamp(video_frame,
                                            ten_value_get_int64(value, err));
          success = ten_error_is_success(err);
        } else if (!strcmp(TEN_STR_WIDTH,
                           ten_string_get_raw_str(&item->obj_item_str))) {
          ten_raw_video_frame_set_width(video_frame,
                                        ten_value_get_int32(value, err));
          success = ten_error_is_success(err);
        } else if (!strcmp(TEN_STR_HEIGHT,
                           ten_string_get_raw_str(&item->obj_item_str))) {
          ten_raw_video_frame_set_height(video_frame,
                                         ten_value_get_int32(value, err));
          success = ten_error_is_success(err);
        }
        break;
      }

      default:
        break;
    }
  }

  if (use_tmp_err) {
    ten_error_deinit(&tmp_err);
  }

  return success;
}

ten_value_t *ten_raw_video_frame_peek_ten_property(ten_msg_t *self,
                                                   ten_list_t *paths,
                                                   ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self), "Invalid argument.");
  TEN_ASSERT(paths && ten_list_check_integrity(paths),
             "path should not be empty.");

  ten_value_t *result = NULL;

  ten_error_t tmp_err;
  bool use_tmp_err = false;
  if (!err) {
    use_tmp_err = true;
    ten_error_init(&tmp_err);
    err = &tmp_err;
  }

  ten_video_frame_t *video_frame = (ten_video_frame_t *)self;

  ten_list_foreach (paths, item_iter) {
    ten_value_path_item_t *item = ten_ptr_listnode_get(item_iter.node);
    TEN_ASSERT(item, "Invalid argument.");

    switch (item->type) {
      case TEN_VALUE_PATH_ITEM_TYPE_OBJECT_ITEM: {
        if (!strcmp(TEN_STR_PIXEL_FMT,
                    ten_string_get_raw_str(&item->obj_item_str))) {
          result = &video_frame->pixel_fmt;
        } else if (!strcmp(TEN_STR_TIMESTAMP,
                           ten_string_get_raw_str(&item->obj_item_str))) {
          result = &video_frame->timestamp;
        } else if (!strcmp(TEN_STR_WIDTH,
                           ten_string_get_raw_str(&item->obj_item_str))) {
          result = &video_frame->width;
        } else if (!strcmp(TEN_STR_HEIGHT,
                           ten_string_get_raw_str(&item->obj_item_str))) {
          result = &video_frame->height;
        } else if (!strcmp(TEN_STR_IS_EOF,
                           ten_string_get_raw_str(&item->obj_item_str))) {
          result = &video_frame->is_eof;
        } else if (!strcmp(TEN_STR_DATA,
                           ten_string_get_raw_str(&item->obj_item_str))) {
          result = &video_frame->data;
        }
        break;
      }

      default:
        break;
    }
  }

  if (use_tmp_err) {
    ten_error_deinit(&tmp_err);
  }

  return result;
}