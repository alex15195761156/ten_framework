//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

typedef enum TEN_AUDIO_FRAME_FIELD {
  TEN_AUDIO_FRAME_FIELD_MSGHDR,

  TEN_AUDIO_FRAME_FIELD_TIMESTAMP,
  TEN_AUDIO_FRAME_FIELD_SAMPLE_RATE,
  TEN_AUDIO_FRAME_FIELD_BYTES_PER_SAMPLE,
  TEN_AUDIO_FRAME_FIELD_SAMPLES_PER_CHANNEL,
  TEN_AUDIO_FRAME_FIELD_NUMBER_OF_CHANNEL,
  TEN_AUDIO_FRAME_FIELD_DATA_FMT,
  TEN_AUDIO_FRAME_FIELD_DATA,
  TEN_AUDIO_FRAME_FIELD_LINE_SIZE,

  TEN_AUDIO_FRAME_FIELD_LAST,
} TEN_AUDIO_FRAME_FIELD;