#
# This file is part of the TEN Framework project.
# Licensed under the Apache License, Version 2.0.
# See the LICENSE file for more information.
#
from ten import (
    Addon,
    Extension,
    register_addon_as_extension,
    TenEnv,
    Cmd,
    StatusCode,
    CmdResult,
    PixelFmt,
    VideoFrame,
)
from PIL import Image, ImageFilter


class PilDemoExtension(Extension):
    def on_init(self, ten_env: TenEnv) -> None:
        print("PilDemoExtension on_init")
        ten_env.on_init_done()

    def on_start(self, ten_env: TenEnv) -> None:
        print("PilDemoExtension on_start")
        ten_env.on_start_done()

    def on_stop(self, ten_env: TenEnv) -> None:
        print("PilDemoExtension on_stop")
        ten_env.on_stop_done()

    def on_deinit(self, ten_env: TenEnv) -> None:
        print("PilDemoExtension on_deinit")
        ten_env.on_deinit_done()

    def on_cmd(self, ten_env: TenEnv, cmd: Cmd) -> None:
        print("PilDemoExtension on_cmd")
        cmd_json = cmd.to_json()
        print("PilDemoExtension on_cmd json: " + cmd_json)

        cmd_result = CmdResult.create(StatusCode.OK)
        cmd_result.set_property_string("detail", "success")
        ten_env.return_result(cmd_result, cmd)

    def on_video_frame(self, ten_env: TenEnv, video_frame: VideoFrame) -> None:
        print("PilDemoExtension on_video_frame")
        if video_frame.get_pixel_fmt() != PixelFmt.RGBA:
            print("PilDemoExtension on_video_frame, not support pixel format")
            return

        im = Image.frombuffer(
            "RGBA",
            (video_frame.get_width(), video_frame.get_height()),
            video_frame.get_buf(),
        )
        im2 = im.filter(ImageFilter.BLUR)
        im2.save("./blur.png")


@register_addon_as_extension("pil_demo_python")
class PilDemoExtensionAddon(Addon):

    def on_init(self, ten_env: TenEnv) -> None:
        print("PilDemoExtensionAddon on_init")
        ten_env.on_init_done()
        return

    def on_create_instance(self, ten_env: TenEnv, name: str, context) -> None:
        print("PilDemoExtensionAddon on_create_instance")
        ten_env.on_create_instance_done(PilDemoExtension(name), context)

    def on_deinit(self, ten_env: TenEnv) -> None:
        print("PilDemoExtensionAddon on_deinit")
        ten_env.on_deinit_done()
        return