//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "ten_runtime/binding/go/interface/ten/value.h"

#include <stdint.h>
#include <stdlib.h>

#include "include_internal/ten_runtime/binding/go/internal/common.h"
#include "include_internal/ten_runtime/binding/go/value/value.h"
#include "include_internal/ten_utils/value/value_smart_ptr.h"
#include "ten_runtime/binding/go/interface/ten/common.h"
#include "ten_runtime/common/errno.h"
#include "ten_utils/lib/alloc.h"
#include "ten_utils/lib/buf.h"
#include "ten_utils/lib/error.h"
#include "ten_utils/lib/json.h"
#include "ten_utils/lib/smart_ptr.h"
#include "ten_utils/lib/string.h"
#include "ten_utils/macro/check.h"
#include "ten_utils/value/type.h"
#include "ten_utils/value/type_operation.h"
#include "ten_utils/value/value.h"
#include "ten_utils/value/value_get.h"
#include "ten_utils/value/value_is.h"

// @{
// TODO(Liu): Deprecated.

ten_go_handle_t tenGoCreateValue(ten_go_value_t *);

void tenGoUnrefObj(ten_go_handle_t);

// The definition of tenUnpinGoPointer is in GO world, and tenUnpinGoPointer is
// exported to C. So we need to declare it, then it can be called from C to GO.
//
// Before a GO pointer is set as a property of a msg or ten instance, it will be
// pinned into the handle map in GO world. And the handle id pointing to the GO
// pointer will be set as the property value, not the GO pointer itself. When
// the msg or ten instance has been reclaimed by TEN runtime, the GO pointer
// must be unpinned from the handle map to avoid memory leak. This function is
// used to unpinned the GO pointer.
void tenUnpinGoPointer(ten_go_handle_t);

bool ten_go_value_check_integrity(ten_go_value_t *self) {
  TEN_ASSERT(self, "Should not happen.");

  if (ten_signature_get(&self->signature) != TEN_GO_VALUE_SIGNATURE) {
    return false;
  }

  return true;
}

ten_go_handle_t ten_go_value_go_handle(ten_go_value_t *self) {
  TEN_ASSERT(self, "Should not happen.");

  return self->bridge.go_instance;
}

ten_value_t *ten_go_value_c_value(ten_go_value_t *self) {
  TEN_ASSERT(self, "Should not happen.");

  return self->c_value;
}

static void ten_go_value_destroy_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  if (self->own) {
    ten_value_destroy(self->c_value);
  }

  TEN_FREE(self);
}

static ten_go_value_t *ten_go_create_empty_value(void) {
  ten_go_value_t *value_bridge =
      (ten_go_value_t *)TEN_MALLOC(sizeof(ten_go_value_t));
  TEN_ASSERT(value_bridge, "Failed to allocate memory.");

  ten_signature_set(&value_bridge->signature, TEN_GO_VALUE_SIGNATURE);
  value_bridge->bridge.go_instance = tenGoCreateValue(value_bridge);

  value_bridge->bridge.sp_ref_by_go =
      ten_shared_ptr_create(value_bridge, ten_go_value_destroy_v1);
  value_bridge->bridge.sp_ref_by_c = NULL;

  return value_bridge;
}

ten_go_handle_t ten_go_wrap_value(ten_value_t *c_value, bool own) {
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_go_value_t *value_bridge = ten_go_create_empty_value();
  value_bridge->c_value = c_value;
  value_bridge->own = own;

  return value_bridge->bridge.go_instance;
}

ten_go_handle_t ten_go_value_create_int8(int8_t v) {
  ten_value_t *value = ten_value_create_int8(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_int16(int16_t v) {
  ten_value_t *value = ten_value_create_int16(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_int32(int32_t v) {
  ten_value_t *value = ten_value_create_int32(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_int64(int64_t v) {
  ten_value_t *value = ten_value_create_int64(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_uint8(uint8_t v) {
  ten_value_t *value = ten_value_create_uint8(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_uint16(uint16_t v) {
  ten_value_t *value = ten_value_create_uint16(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_uint32(uint32_t v) {
  ten_value_t *value = ten_value_create_uint32(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_uint64(uint64_t v) {
  ten_value_t *value = ten_value_create_uint64(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_float32(float v) {
  ten_value_t *value = ten_value_create_float32(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_float64(double v) {
  ten_value_t *value = ten_value_create_float64(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_string(const char *v) {
  ten_value_t *value = ten_value_create_string(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_bool(bool v) {
  ten_value_t *value = ten_value_create_bool(v);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_byte_array(void *buf, int len) {
  ten_buf_t buf_ = TEN_BUF_STATIC_INIT_WITH_DATA_OWNED(buf, len);
  ten_value_t *value = ten_value_create_buf_with_move(buf_);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

static void go_ref_unlink_handler(ten_go_handle_t *v) {
  tenGoUnrefObj(*v);

  TEN_FREE(v);
}

ten_go_handle_t ten_go_value_create_ptr(ten_go_handle_t v) {
  // The base type of ten_go_handle_t is uintptr_t, which is the same as void*.
  // So in general, we can create a pointer value using the ten_go_handle_t
  // directly in C, without creating a new ten_go_handle_t*. Ex:
  //
  //   ten_value_create_ptr((void*)v, NULL, NULL, NULL);
  //
  // However, the parameter `v` here is not an ordinary pointer actually, it is
  // an index pointing to a GO pointer in the handle map. If the upstream is
  // some extension written in C, it's dangerous to interpret it as an address,
  // as the address might be accessible. So creating a new ten_go_handle_t* is
  // much more better.
  ten_go_handle_t *go_ref_ptr = TEN_MALLOC(sizeof(ten_go_handle_t));
  TEN_ASSERT(go_ref_ptr, "Failed to allocate memory.");

  *go_ref_ptr = v;

  ten_shared_ptr_t *go_ref_id_shared =
      ten_shared_ptr_create(go_ref_ptr, go_ref_unlink_handler);
  TEN_ASSERT(go_ref_id_shared, "Should not happen.");

  ten_value_t *value = ten_value_create_ptr(
      ten_shared_ptr_clone(go_ref_id_shared), ten_value_construct_for_smart_ptr,
      ten_value_copy_for_smart_ptr, ten_value_destruct_for_smart_ptr);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  ten_shared_ptr_destroy(go_ref_id_shared);

  return ten_go_wrap_value(value, true);
}

ten_go_handle_t ten_go_value_create_from_json(const char *json_str) {
  TEN_ASSERT(json_str, "Should not happen.");

  ten_json_t *json = ten_json_from_string(json_str, NULL);
  TEN_ASSERT(json, "Should not happen.");

  ten_value_t *value = ten_value_from_json(json);
  TEN_ASSERT(value && ten_value_check_integrity(value), "Should not happen.");

  return ten_go_wrap_value(value, true);
}

int8_t ten_go_value_get_int8_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  int8_t value = ten_value_get_int8(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

int16_t ten_go_value_get_int16_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  int16_t value = ten_value_get_int16(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

int32_t ten_go_value_get_int32_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  int32_t value = ten_value_get_int32(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

int64_t ten_go_value_get_int64_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  int64_t value = ten_value_get_int64(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

uint8_t ten_go_value_get_uint8_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  uint8_t value = ten_value_get_uint8(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

uint16_t ten_go_value_get_uint16_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  uint16_t value = ten_value_get_uint16(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

uint32_t ten_go_value_get_uint32_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  uint32_t value = ten_value_get_uint32(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

uint64_t ten_go_value_get_uint64_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  uint64_t value = ten_value_get_uint64(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

float ten_go_value_get_float32_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  float value = ten_value_get_float32(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

double ten_go_value_get_float64_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  double value = ten_value_get_float64(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

const char *ten_go_value_get_string_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  return ten_value_peek_c_str(c_value);
}

bool ten_go_value_get_bool_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  bool value = ten_value_get_bool(c_value, &err);

  ten_error_deinit(&err);

  return value;
}

void *ten_go_value_get_buf_data(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  return ten_value_peek_buf(c_value) ? ten_value_peek_buf(c_value)->data : NULL;
}

int ten_go_value_get_buf_size(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  return ten_value_peek_buf(c_value) ? (int)ten_value_peek_buf(c_value)->size
                                     : 0;
}

ten_go_handle_t ten_go_value_get_ptr_v1(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  ten_shared_ptr_t *go_ref_shared_ptr = ten_value_get_ptr(c_value, &err);

  ten_error_deinit(&err);

  ten_go_handle_t *go_ref = ten_shared_ptr_get_data(go_ref_shared_ptr);

  return *go_ref;
}

const char *ten_go_value_to_json(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_value_t *c_value = self->c_value;
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_json_t *json = ten_value_to_json(c_value);
  TEN_ASSERT(json, "Failed to convert value to json.");

  bool must_free = false;
  const char *json_str = ten_json_to_string(json, NULL, &must_free);
  TEN_ASSERT(json_str, "Failed to get JSON string from JSON.");

  ten_json_destroy(json);
  return json_str;
}

void ten_go_value_finalize(ten_go_value_t *self) {
  TEN_ASSERT(self && ten_go_value_check_integrity(self), "Should not happen.");

  ten_go_bridge_destroy_go_part(&self->bridge);
}

// @}

void ten_go_ten_value_get_type_and_size(ten_value_t *self, uint8_t *type,
                                        uintptr_t *size) {
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");

  TEN_TYPE prop_type = ten_value_get_type(self);
  *type = prop_type;

  switch (prop_type) {
    case TEN_TYPE_BUF: {
      ten_buf_t *buf = ten_value_peek_buf(self);
      *size = buf ? ten_buf_get_size(buf) : 0;
      break;
    }

    case TEN_TYPE_STRING: {
      const char *str = ten_value_peek_string(self);
      TEN_ASSERT(str, "Should not happen.");

      *size = strlen(str);
      break;
    }

    default:
      // For other types, the size is always 0.
      break;
  }
}

void ten_go_ten_value_get_string(ten_value_t *self, void *value,
                                 ten_go_status_t *status) {
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value && status, "Should not happen.");

  if (!ten_value_is_string(self)) {
    ten_go_status_set_errno(status, TEN_ERRNO_GENERIC);
    return;
  }

  const char *str_value = ten_value_peek_string(self);
  TEN_ASSERT(str_value, "Should not happen");

  // The value is a pointer to a GO slice which has no space for the null
  // terminator.
  //
  // If we use strcpy here, the byte next to the GO slice will be set to '\0',
  // as strcpy copies the null terminator to the dest. If the memory space has
  // been allocated for other variables in GO world, ex: a GO string, the first
  // byte of the variable will be modified. The memory layout is as follows.
  //
  // We prefer to use `memcpy` rather than `strncpy`, because `memcpy` has a
  // performance optimization on some platforms.
  memcpy(value, str_value, strlen(str_value));
}

void ten_go_ten_value_get_buf(ten_value_t *self, void *value,
                              ten_go_status_t *status) {
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value && status, "Should not happen.");

  if (!ten_value_is_buf(self)) {
    ten_go_status_set_errno(status, TEN_ERRNO_GENERIC);
    return;
  }

  ten_buf_t *buf = ten_value_peek_buf(self);
  if (buf) {
    memcpy(value, ten_buf_get_data(buf), ten_buf_get_size(buf));
  }
}

void ten_go_ten_value_get_ptr(ten_value_t *self, ten_go_handle_t *value,
                              ten_go_status_t *status) {
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value && status, "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  ten_shared_ptr_t *handle_ptr = ten_value_get_ptr(self, &err);
  if (ten_error_is_success(&err)) {
    void *go_ref = ten_shared_ptr_get_data(handle_ptr);
    *value = (ten_go_handle_t)go_ref;
  } else {
    ten_go_status_from_error(status, &err);
  }

  ten_error_deinit(&err);
}

ten_value_t *ten_go_ten_value_create_buf(void *value, int value_len) {
  TEN_ASSERT(value, "Should not happen.");

  ten_buf_t buf;
  ten_buf_init_with_owned_data(&buf, value_len);

  memcpy(buf.data, value, value_len);

  ten_value_t *c_value = ten_value_create_buf_with_move(buf);
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  return c_value;
}

static void ten_go_handle_unpin_from_go(void *v) {
  ten_go_handle_t handle = (ten_go_handle_t)v;
  tenUnpinGoPointer(handle);
}

ten_value_t *ten_go_ten_value_create_ptr(ten_go_handle_t value) {
  TEN_ASSERT(value > 0, "Should not happen.");

  // The base type of ten_go_handle_t is uintptr_t, whose bit size is same as
  // void*. It's ok to reinterpret the `value` as a void*. However, the `value`
  // is not an ordinary pointer actually, it is an index pointing to a GO
  // pointer in the handle map in the GO world. So the reinterpreted pointer can
  // not be dereferenced.
  void *handle = (void *)value;  // NOLINT(performance-no-int-to-ptr)

  // The reason why we need to create a shared_ptr here is as follows.
  //
  // A ten_go_handle_t is a reference to a GO pointer in the handle map in the
  // GO world. The handle map is used to pin the GO pointer when it is used as a
  // property of a msg.
  //
  // When extension A sets a GO pointer as a property of a msg, the GO pointer
  // will be pinned into the handle map. The relationship is as follows.
  //
  //                                 HandleMap (GO)
  //                                  <key, value>
  //                                    ^     |
  //                                    |     +--> A GO pointer.
  //                          +- equal -+
  //                          |
  //   msg.SetProperty(key, value)
  //                          |
  //                          +--> A ten_go_handle_t.
  //
  // Imagine that extension B is the upstream of A, and it gets the GO pointer
  // from the msg. The relationship is as follows.
  //
  //                                 HandleMap (GO)
  //                                  <key, value>
  //                                    ^     |
  //                                    |     +--> A GO pointer.
  //                          +- equal -+
  //                          |
  //                        value = msg.GetProperty(key)
  //
  // So the GO pointer in the handle map _must_ be pinned until extension B has
  // handed the msg over to TEN runtime. Thus, the GO pointer can only be
  // unpinned from C to GO, but not be unpinned once the msg is sent out from
  // extension A. And if A has more than one consumer, the GO pointer must be
  // pinned until all the consumers have handed the msgs over to TEN runtime.
  // That's what `ten_go_handle_unpin_from_go` will do.
  ten_shared_ptr_t *handle_ptr =
      ten_shared_ptr_create(handle, ten_go_handle_unpin_from_go);
  TEN_ASSERT(handle_ptr, "Should not happen.");

  ten_value_t *c_value = ten_value_create_ptr(
      ten_shared_ptr_clone(handle_ptr), ten_value_construct_for_smart_ptr,
      ten_value_copy_for_smart_ptr, ten_value_destruct_for_smart_ptr);
  TEN_ASSERT(c_value && ten_value_check_integrity(c_value),
             "Should not happen.");

  ten_shared_ptr_destroy(handle_ptr);

  return c_value;
}

bool ten_go_ten_value_to_json(ten_value_t *self, uintptr_t *json_str_len,
                              const char **json_str, ten_go_status_t *status) {
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(json_str_len && json_str && status, "Should not happen.");

  ten_json_t *json = ten_value_to_json(self);
  if (json == NULL) {
    ten_string_t err_msg;
    ten_string_init_formatted(&err_msg, "the property type is %s",
                              ten_type_to_string(ten_value_get_type(self)));

    ten_go_status_set(status, TEN_ERRNO_GENERIC,
                      ten_string_get_raw_str(&err_msg));

    ten_string_deinit(&err_msg);

    return false;
  }

  // The json bytes are allocated by ten_json_to_string, and will be freed after
  // the GO slice is created. The GO slice must be created in GO world, as the
  // underlying buffer to the slice must be in the GO heap, that's why we have
  // to return the json bytes and the length to GO world first. And then create
  // a slice in GO world, and copy the json bytes to the slice and destroy the
  // json bytes. It will be done in ten_go_copy_c_str_to_slice_and_free.
  bool must_free = false;
  *json_str = ten_json_to_string(json, NULL, &must_free);
  ten_json_destroy(json);

  *json_str_len = strlen(*json_str);

  return true;
}

// Note that value_addr is the bit pattern of the pointer to ten_value_t, not a
// value bridge. There is no bridge for ten_value_t, as no GO object is created
// for it.
static ten_value_t *ten_go_value_reinterpret(uintptr_t value_addr) {
  TEN_ASSERT(value_addr > 0, "Should not happen.");

  // NOLINTNEXTLINE(performance-no-int-to-ptr)
  ten_value_t *self = (ten_value_t *)value_addr;
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");

  return self;
}

ten_go_status_t ten_go_value_get_int8(uintptr_t value_addr, int8_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_int8(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  // As the ten_value_t is cloned in the previous stage, we need to destroy it
  // here.
  //
  // TODO(Liu): It’s quite hard to understand why a destroy suddenly appears
  // here, which shows the tight coupling between these functions and other
  // execution contexts. There's a need for decoupling.
  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_int16(uintptr_t value_addr, int16_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_int16(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_int32(uintptr_t value_addr, int32_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_int32(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_int64(uintptr_t value_addr, int64_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_int64(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_uint8(uintptr_t value_addr, uint8_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_uint8(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_uint16(uintptr_t value_addr, uint16_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_uint16(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_uint32(uintptr_t value_addr, uint32_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_uint32(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_uint64(uintptr_t value_addr, uint64_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_uint64(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_float32(uintptr_t value_addr, float *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_float32(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_float64(uintptr_t value_addr, double *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_float64(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_bool(uintptr_t value_addr, bool *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_error_t err;
  ten_error_init(&err);

  *value = ten_value_get_bool(self, &err);

  ten_go_status_from_error(&status, &err);
  ten_error_deinit(&err);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_string(uintptr_t value_addr, void *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_go_ten_value_get_string(self, value, &status);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_buf(uintptr_t value_addr, void *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_go_ten_value_get_buf(self, value, &status);

  ten_value_destroy(self);

  return status;
}

ten_go_status_t ten_go_value_get_ptr(uintptr_t value_addr,
                                     ten_go_handle_t *value) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");
  TEN_ASSERT(value, "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  ten_go_ten_value_get_ptr(self, value, &status);

  ten_value_destroy(self);

  return status;
}

void ten_go_value_destroy(uintptr_t value_addr) {
  ten_value_t *self = ten_go_value_reinterpret(value_addr);
  TEN_ASSERT(self && ten_value_check_integrity(self), "Should not happen.");

  ten_value_destroy(self);
}