//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "ten_utils/container/list_node.h"

#include <stdlib.h>
#include <string.h>

#include "ten_utils/lib/alloc.h"
#include "ten_utils/macro/check.h"

bool ten_listnode_check_integrity(ten_listnode_t *self) {
  TEN_ASSERT(self, "Invalid argument.");

  if (ten_signature_get(&self->signature) !=
      (ten_signature_t)TEN_LISTNODE_SIGNATURE) {
    return false;
  }
  return true;
}

void ten_listnode_init(ten_listnode_t *self, void *destroy) {
  TEN_ASSERT(self, "Invalid argument.");

  ten_signature_set(&self->signature, (ten_signature_t)TEN_LISTNODE_SIGNATURE);
  self->destroy = destroy;
  self->next = NULL;
  self->prev = NULL;
}

void ten_listnode_destroy(ten_listnode_t *self) {
  TEN_ASSERT(self && ten_listnode_check_integrity(self), "Invalid argument.");

  if (self->destroy) {
    self->destroy(self);
  }
  ten_free(self);
}

void ten_listnode_destroy_only(ten_listnode_t *self) {
  TEN_ASSERT(self && ten_listnode_check_integrity(self), "Invalid argument.");
  ten_free(self);
}