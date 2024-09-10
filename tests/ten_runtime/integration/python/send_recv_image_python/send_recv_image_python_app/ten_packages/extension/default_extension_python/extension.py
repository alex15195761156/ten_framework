#
# This file is part of the TEN Framework project.
# See https://github.com/TEN-framework/ten_framework/LICENSE for license
# information.
#
import time
from ten import (
    Extension,
    TenEnv,
    Cmd,
    VideoFrame,
    StatusCode,
    CmdResult,
    PixelFmt,
)
from PIL import Image


class DefaultExtension(Extension):
    def on_init(self, ten_env: TenEnv) -> None:
        print("DefaultExtension on_init")
        ten_env.init_property_from_json('{"testKey": "testValue"}')
        ten_env.on_init_done()

    def on_start(self, ten_env: TenEnv) -> None:
        print("DefaultExtension on_start")

        im = Image.open("../test_data/tiger.jpg")
        self.pixels = im.convert("RGBA")
        self.image_bytes = self.pixels.tobytes()

        ten_env.on_start_done()

    def on_cmd(self, ten_env: TenEnv, cmd: Cmd) -> None:
        cmd_json = cmd.to_json()
        print("DefaultExtension on_cmd json: " + cmd_json)

        assert hasattr(self, "request_cmd") != True

        self.request_cmd = cmd

        new_image = VideoFrame.create("image")
        new_image.alloc_buf(len(self.image_bytes))
        new_image.set_width(self.pixels.width)
        new_image.set_height(self.pixels.height)
        new_image.set_pixel_fmt(PixelFmt.RGBA)
        new_image.set_timestamp(int(time.time() * 1000))
        buf = new_image.lock_buf()

        buf[0 : len(self.image_bytes)] = self.image_bytes

        new_image.unlock_buf(buf)

        width = new_image.get_width()
        height = new_image.get_height()
        pixel_fmt = new_image.get_pixel_fmt()
        timestamp = new_image.get_timestamp()
        print(
            "DefaultExtension send video frame, width: %d, height: %d, pixel_fmt: %d, timestamp: %d"
            % (width, height, pixel_fmt, timestamp)
        )

        ten_env.send_video_frame(new_image)

    def on_video_frame(self, ten_env: TenEnv, video_frame: VideoFrame) -> None:
        print("DefaultExtension on_video_frame")

        assert hasattr(self, "request_cmd") == True

        assert video_frame.get_height() == self.pixels.height
        assert video_frame.get_width() == self.pixels.width
        assert video_frame.get_pixel_fmt() == PixelFmt.RGBA

        buf = video_frame.get_buf()
        assert buf[0 : len(self.image_bytes)] == self.image_bytes

        cmd_result = CmdResult.create(StatusCode.OK)
        cmd_result.set_property_string("detail", "success")
        ten_env.return_result(cmd_result, self.request_cmd)