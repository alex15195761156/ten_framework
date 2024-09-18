//
// Copyright © 2024 Agora
// This file is part of TEN Framework, an open source project.
// Licensed under the Apache License, Version 2.0, with certain conditions.
// Refer to the "LICENSE" file in the root directory for more information.
//
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <string>
#include <utility>

#include "ten_runtime/binding/common.h"
#include "ten_runtime/binding/cpp/internal/common.h"
#include "ten_runtime/binding/cpp/internal/msg/audio_frame.h"
#include "ten_runtime/binding/cpp/internal/msg/cmd/cmd.h"
#include "ten_runtime/binding/cpp/internal/msg/cmd/stop_graph.h"
#include "ten_runtime/binding/cpp/internal/msg/data.h"
#include "ten_runtime/binding/cpp/internal/msg/video_frame.h"
#include "ten_runtime/binding/cpp/internal/ten_env.h"
#include "ten_runtime/extension/extension.h"
#include "ten_runtime/msg/cmd/stop_graph/cmd.h"
#include "ten_runtime/msg/msg.h"
#include "ten_runtime/ten_env/ten_env.h"
#include "ten_utils/lib/json.h"
#include "ten_utils/lib/smart_ptr.h"
#include "ten_utils/macro/check.h"

using ten_json_t = ::ten_json_t;
using ten_env_t = struct ten_env_t;
using ten_extension_t = struct ten_extension_t;

namespace ten {

class ten_env_t;
class extension_group_t;

class extension_t {
 public:
  virtual ~extension_t() {
    TEN_ASSERT(c_extension, "Should not happen.");
    ten_extension_destroy(c_extension);

    TEN_ASSERT(cpp_ten_env, "Should not happen.");
    delete cpp_ten_env;
  }

  // @{
  extension_t(const extension_t &) = delete;
  extension_t(extension_t &&) = delete;
  extension_t &operator=(const extension_t &) = delete;
  extension_t &operator=(const extension_t &&) = delete;
  // @}

  // @{
  // Internal use only.
  ::ten_extension_t *get_c_extension() const { return c_extension; }
  // @}

 protected:
  explicit extension_t(const std::string &name)
      :  // In order to keep type safety in C++, the type of the 'ten'
         // parameters of these 4 functions are indeed 'ten::ten_env_t'.
         // However, these callback functions are called from the C language
         // world, and in order to keep type safety in the C language world, it
         // better uses reinterpret_cast<> here, rather than change the type of
         // the second parameter of these callback functions from '::ten_env_t
         // *' to 'void *'
        c_extension(::ten_extension_create(
            name.c_str(),
            reinterpret_cast<ten_extension_on_configure_func_t>(
                &proxy_on_configure),
            reinterpret_cast<ten_extension_on_init_func_t>(&proxy_on_init),
            reinterpret_cast<ten_extension_on_start_func_t>(&proxy_on_start),
            reinterpret_cast<ten_extension_on_stop_func_t>(&proxy_on_stop),
            reinterpret_cast<ten_extension_on_deinit_func_t>(&proxy_on_deinit),
            reinterpret_cast<ten_extension_on_cmd_func_t>(&proxy_on_cmd),
            reinterpret_cast<ten_extension_on_data_func_t>(&proxy_on_data),
            reinterpret_cast<ten_extension_on_audio_frame_func_t>(
                &proxy_on_audio_frame),
            reinterpret_cast<ten_extension_on_video_frame_func_t>(
                &proxy_on_video_frame),
            nullptr)) {
    TEN_ASSERT(c_extension, "Should not happen.");

    ten_binding_handle_set_me_in_target_lang(
        reinterpret_cast<ten_binding_handle_t *>(c_extension),
        static_cast<void *>(this));

    cpp_ten_env = new ten_env_t(ten_extension_get_ten(c_extension));
    TEN_ASSERT(cpp_ten_env, "Should not happen.");
  }

  virtual void on_configure(ten_env_t &ten_env) { ten_env.on_configure_done(); }

  virtual void on_init(ten_env_t &ten_env) { ten_env.on_init_done(); }

  virtual void on_start(ten_env_t &ten_env) { ten_env.on_start_done(); }

  virtual void on_stop(ten_env_t &ten_env) { ten_env.on_stop_done(); }

  virtual void on_deinit(ten_env_t &ten_env) { ten_env.on_deinit_done(); }

  // Use std::shared_ptr to enable the C++ extension to save the received C++
  // messages and use it later. And use 'const shared_ptr&' to indicate that
  // the C++ extension "might" take a copy and share ownership of the cmd.

  virtual void on_cmd(ten_env_t &ten_env, std::unique_ptr<cmd_t> cmd) {
    auto cmd_result = ten::cmd_result_t::create(TEN_STATUS_CODE_OK);
    cmd_result->set_property("detail", "default");
    ten_env.return_result(std::move(cmd_result), std::move(cmd));
  }

  virtual void on_data(ten_env_t &ten_env, std::unique_ptr<data_t> data) {}

  virtual void on_audio_frame(ten_env_t &ten_env,
                              std::unique_ptr<audio_frame_t> frame) {}

  virtual void on_video_frame(ten_env_t &ten_env,
                              std::unique_ptr<video_frame_t> frame) {}

 private:
  friend class ten_env_t;
  friend class extension_group_t;

  using cpp_extension_on_cmd_func_t =
      void (extension_t:: *)(ten_env_t &, std::unique_ptr<cmd_t>);

  static void issue_stop_graph_cmd(ten_env_t &ten_env) {
    // Issue a 'close engine' command, and in order to gain the maximum
    // performance, we use C code directly here.
    ten_shared_ptr_t *stop_graph_cmd = ten_cmd_stop_graph_create();
    TEN_ASSERT(stop_graph_cmd, "Should not happen.");

    ten_msg_clear_and_set_dest(stop_graph_cmd, "localhost", nullptr, nullptr,
                               nullptr, nullptr, nullptr);
    ten_env_send_cmd(ten_env.get_c_ten_env(), stop_graph_cmd, nullptr, nullptr,
                     nullptr);
    ten_shared_ptr_destroy(stop_graph_cmd);
  }

  static void proxy_on_configure(ten_extension_t *extension,
                                 ::ten_env_t *ten_env) {
    TEN_ASSERT(extension && ten_env, "Should not happen.");

    auto *cpp_extension =
        static_cast<extension_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(extension)));
    auto *cpp_ten_env =
        static_cast<ten_env_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(ten_env)));

    cpp_extension->invoke_cpp_extension_on_configure(*cpp_ten_env);
  }

  static void proxy_on_init(ten_extension_t *extension, ::ten_env_t *ten_env) {
    TEN_ASSERT(extension && ten_env, "Should not happen.");

    auto *cpp_extension =
        static_cast<extension_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(extension)));
    auto *cpp_ten_env =
        static_cast<ten_env_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(ten_env)));

    cpp_extension->invoke_cpp_extension_on_init(*cpp_ten_env);
  }

  static void proxy_on_start(ten_extension_t *extension, ::ten_env_t *ten_env) {
    TEN_ASSERT(extension && ten_env, "Should not happen.");

    auto *cpp_extension =
        static_cast<extension_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(extension)));
    auto *cpp_ten_env =
        static_cast<ten_env_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(ten_env)));

    cpp_extension->invoke_cpp_extension_on_start(*cpp_ten_env);
  }

  static void proxy_on_stop(ten_extension_t *extension, ::ten_env_t *ten_env) {
    TEN_ASSERT(extension && ten_env, "Should not happen.");

    auto *cpp_extension =
        static_cast<extension_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(extension)));
    auto *cpp_ten_env =
        static_cast<ten_env_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(ten_env)));

    cpp_extension->invoke_cpp_extension_on_stop(*cpp_ten_env);
  }

  static void proxy_on_deinit(ten_extension_t *extension,
                              ::ten_env_t *ten_env) {
    TEN_ASSERT(extension && ten_env, "Should not happen.");

    auto *cpp_extension =
        static_cast<extension_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(extension)));
    auto *cpp_ten_env =
        static_cast<ten_env_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(ten_env)));

    cpp_extension->invoke_cpp_extension_on_deinit(*cpp_ten_env);
  }

  static void proxy_on_cmd_internal(ten_extension_t *extension,
                                    ::ten_env_t *ten_env, ten_shared_ptr_t *cmd,
                                    cpp_extension_on_cmd_func_t on_cmd_func);

  // This function would be called when the extension does _not_ enable the
  // command binding mechanism.
  static void proxy_on_cmd(ten_extension_t *extension, ::ten_env_t *ten_env,
                           ten_shared_ptr_t *cmd) {
    proxy_on_cmd_internal(extension, ten_env, cmd, &extension_t::on_cmd);
  }

  static void proxy_on_data(ten_extension_t *extension, ::ten_env_t *ten_env,
                            ten_shared_ptr_t *data) {
    TEN_ASSERT(extension && ten_env, "Should not happen.");

    auto *cpp_extension =
        static_cast<extension_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(extension)));
    auto *cpp_ten_env =
        static_cast<ten_env_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(ten_env)));

    auto cpp_data_ptr = std::make_unique<data_t>(
        // Clone a C shared_ptr to be owned by the C++ instance.
        ten_shared_ptr_clone(data));

    cpp_extension->invoke_cpp_extension_on_data(*cpp_ten_env,
                                                std::move(cpp_data_ptr));
  }

  static void proxy_on_audio_frame(ten_extension_t *extension,
                                   ::ten_env_t *ten_env,
                                   ten_shared_ptr_t *frame) {
    TEN_ASSERT(extension && ten_env && frame, "Should not happen.");

    auto *cpp_extension =
        static_cast<extension_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(extension)));
    auto *cpp_ten_env =
        static_cast<ten_env_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(ten_env)));

    auto cpp_frame_ptr = std::make_unique<audio_frame_t>(
        // Clone a C shared_ptr to be owned by the C++ instance.
        ten_shared_ptr_clone(frame));

    cpp_extension->invoke_cpp_extension_on_audio_frame(
        *cpp_ten_env, std::move(cpp_frame_ptr));
  }

  static void proxy_on_video_frame(ten_extension_t *extension,
                                   ::ten_env_t *ten_env,
                                   ten_shared_ptr_t *frame) {
    TEN_ASSERT(extension && ten_env && frame, "Should not happen.");

    auto *cpp_extension =
        static_cast<extension_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(extension)));
    auto *cpp_ten_env =
        static_cast<ten_env_t *>(ten_binding_handle_get_me_in_target_lang(
            reinterpret_cast<ten_binding_handle_t *>(ten_env)));

    auto cpp_frame_ptr = std::make_unique<video_frame_t>(
        // Clone a C shared_ptr to be owned by the C++ instance.
        ten_shared_ptr_clone(frame));

    cpp_extension->invoke_cpp_extension_on_video_frame(
        *cpp_ten_env, std::move(cpp_frame_ptr));
  }

  void invoke_cpp_extension_on_configure(ten_env_t &ten_env) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      on_configure(ten_env);
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_configure(), %s", e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGW("Caught a exception of type '%s' in extension on_configure().",
               curr_exception_type_name().c_str());

      issue_stop_graph_cmd(ten_env);
    }
  }

  void invoke_cpp_extension_on_init(ten_env_t &ten_env) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      on_init(ten_env);
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_init(), %s", e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGW("Caught a exception of type '%s' in extension on_init().",
               curr_exception_type_name().c_str());

      issue_stop_graph_cmd(ten_env);
    }
  }

  void invoke_cpp_extension_on_start(ten_env_t &ten_env) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      on_start(ten_env);
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_start(), %s", e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGW("Caught a exception of type '%s' in extension on_start().",
               curr_exception_type_name().c_str());

      issue_stop_graph_cmd(ten_env);
    }
  }

  void invoke_cpp_extension_on_stop(ten_env_t &ten_env) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      on_stop(ten_env);
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_stop(), %s", e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGD("Caught a exception '%s' in extension on_stop().",
               curr_exception_type_name().c_str());

      issue_stop_graph_cmd(ten_env);
    }
  }

  void invoke_cpp_extension_on_deinit(ten_env_t &ten_env) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      on_deinit(ten_env);
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_deinit(), %s", e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGD("Caught a exception '%s' in extension on_deinit().",
               curr_exception_type_name().c_str());

      issue_stop_graph_cmd(ten_env);
    }
  }

  void invoke_cpp_extension_on_cmd(ten_env_t &ten_env,
                                   std::unique_ptr<cmd_t> cmd,
                                   cpp_extension_on_cmd_func_t on_cmd_func) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      (this->*on_cmd_func)(ten_env, std::move(cmd));
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_cmd(), %s", e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGE("Caught a exception '%s' in extension on_cmd().",
               curr_exception_type_name().c_str());
      issue_stop_graph_cmd(ten_env);
    }
  }

  void invoke_cpp_extension_on_data(ten_env_t &ten_env,
                                    std::unique_ptr<data_t> data) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      on_data(ten_env, std::move(data));
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_data(), %s", e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGD("Caught a exception '%s' in extension on_data().",
               curr_exception_type_name().c_str());

      issue_stop_graph_cmd(ten_env);
    }
  }

  void invoke_cpp_extension_on_audio_frame(
      ten_env_t &ten_env, std::unique_ptr<audio_frame_t> frame) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      on_audio_frame(ten_env, std::move(frame));
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_audio_frame(), %s",
               e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGD("Caught a exception '%s' in extension on_audio_frame().",
               curr_exception_type_name().c_str());

      issue_stop_graph_cmd(ten_env);
    }
  }

  void invoke_cpp_extension_on_video_frame(
      ten_env_t &ten_env, std::unique_ptr<video_frame_t> frame) {
    // The TEN runtime does not use C++ exceptions. The use of try/catch here is
    // merely to intercept any exceptions that might be thrown by the user's app
    // code. If exceptions are disabled during the compilation of the TEN
    // runtime (i.e., with -fno-exceptions), it implies that the extensions used
    // will also not employ exceptions (otherwise it would be unreasonable). In
    // this case, the try/catch blocks become no-ops. Conversely, if exceptions
    // are enabled during compilation, then the try/catch here can intercept all
    // exceptions thrown by user code that are not already caught, serving as a
    // kind of fallback.
    try {
      on_video_frame(ten_env, std::move(frame));
    } catch (std::exception &e) {
      TEN_LOGW("Caught a exception in extension on_video_frame(), %s",
               e.what());

      issue_stop_graph_cmd(ten_env);
    } catch (...) {
      TEN_LOGD("Caught a exception '%s' in extension on_video_frame().",
               curr_exception_type_name().c_str());

      issue_stop_graph_cmd(ten_env);
    }
  }

  ::ten_extension_t *c_extension;
  ten_env_t *cpp_ten_env;
};

}  // namespace ten
