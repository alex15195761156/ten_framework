//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "include_internal/ten_runtime/binding/cpp/ten.h"
#include "ten_utils/lib/thread.h"
#include "ten_utils/lib/time.h"
#include "tests/common/client/cpp/msgpack_tcp.h"
#include "tests/ten_runtime/smoke/extension_test/util/binding/cpp/check.h"

namespace {

class test_extension_1 : public ten::extension_t {
 public:
  explicit test_extension_1(const std::string &name) : ten::extension_t(name) {}

  void on_cmd(ten::ten_env_t &ten_env,
              std::unique_ptr<ten::cmd_t> cmd) override {
    nlohmann::json json = nlohmann::json::parse(cmd->to_json());

    if (json["_ten"]["name"] == "hello_world") {
      ten_env.send_cmd(std::move(cmd));
      return;
    }
  }
};

class test_extension_2 : public ten::extension_t {
 public:
  explicit test_extension_2(const std::string &name) : ten::extension_t(name) {}

  void on_cmd(ten::ten_env_t &ten_env,
              std::unique_ptr<ten::cmd_t> cmd) override {
    nlohmann::json json = nlohmann::json::parse(cmd->to_json());
    if (json["_ten"]["name"] == "hello_world") {
      auto cmd_result = ten::cmd_result_t::create(TEN_STATUS_CODE_OK);
      cmd_result->set_property("detail", "hello world, too");
      ten_env.return_result(std::move(cmd_result), std::move(cmd));
    }
  }
};

class test_app_1 : public ten::app_t {
 public:
  void on_init(ten::ten_env_t &ten_env) override {
    ten::ten_env_internal_accessor_t ten_env_internal_accessor(&ten_env);
    bool rc = ten_env_internal_accessor.init_manifest_from_json(
        // clang-format off
                 R"({
                      "type": "app",
                      "name": "test_app",
                      "version": "0.1.0"
                    })"
        // clang-format on
    );
    ASSERT_EQ(rc, true);

    rc = ten_env.init_property_from_json(
        // clang-format off
                 R"({
                      "_ten": {
                        "uri": "msgpack://127.0.0.1:8001/",
                        "log_level": 1,
                        "predefined_graphs": [{
                           "name": "0",
                           "auto_start": false,
                           "nodes": [{
                              "type": "extension_group",
                              "app": "msgpack://127.0.0.1:8001/",
                              "addon": "default_extension_group",
                              "name": "predefined graph group"
                           }, {
                              "type": "extension",
                              "app": "msgpack://127.0.0.1:8001/",
                              "extension_group": "predefined graph group",
                              "addon": "predefined_graph_multi_app__extension_1",
                              "name": "test extension 1"
                           }, {
                              "type": "extension_group",
                              "app": "msgpack://127.0.0.1:8002/",
                              "addon": "default_extension_group",
                              "name": "predefined graph group"
                           }, {
                              "type": "extension",
                              "app": "msgpack://127.0.0.1:8002/",
                              "extension_group": "predefined graph group",
                              "addon": "predefined_graph_multi_app__extension_2",
                              "name": "test extension 2"
                           }],
                           "connections": [{
                             "app": "msgpack://127.0.0.1:8001/",
                             "extension_group": "predefined graph group",
                             "extension": "test extension 1",
                             "cmd": [{
                               "name": "hello_world",
                               "dest": [{
                                 "app": "msgpack://127.0.0.1:8002/",
                                 "extension_group": "predefined graph group",
                                 "extension": "test extension 2"
                               }]
                             }]
                           }]
                         }]
                       }
                     })"
        // clang-format on
    );
    ASSERT_EQ(rc, true);

    ten_env.on_init_done();
  }
};

class test_app_2 : public ten::app_t {
 public:
  void on_init(ten::ten_env_t &ten_env) override {
    ten_env.init_property_from_json(
        R"({
                      "_ten": {
                        "uri": "msgpack://127.0.0.1:8002/"
                      }
                     })");
    ten_env.on_init_done();
  }
};

void *app_thread_1_main(TEN_UNUSED void *args) {
  auto *app = new test_app_1();
  app->run();
  delete app;

  return nullptr;
}

void *app_thread_2_main(TEN_UNUSED void *args) {
  auto *app = new test_app_2();
  app->run();
  delete app;

  return nullptr;
}

TEN_CPP_REGISTER_ADDON_AS_EXTENSION(predefined_graph_multi_app__extension_1,
                                    test_extension_1);
TEN_CPP_REGISTER_ADDON_AS_EXTENSION(predefined_graph_multi_app__extension_2,
                                    test_extension_2);

}  // namespace

TEST(ExtensionTest, PredefinedGraphMultiApp) {  // NOLINT
  // Start app.
  auto *app_2_thread =
      ten_thread_create("app thread 2", app_thread_2_main, nullptr);
  auto *app_1_thread =
      ten_thread_create("app thread 1", app_thread_1_main, nullptr);

  ten_sleep(300);

  // Create a client and connect to the app.
  auto *client = new ten::msgpack_tcp_client_t("msgpack://127.0.0.1:8001/");

  // Send a user-defined 'hello world' command.
  nlohmann::json resp = client->send_json_and_recv_resp_in_json(
      R"({
         "_ten": {
           "name": "hello_world",
           "seq_id": "137",
           "dest": [{
             "app": "msgpack://127.0.0.1:8001/",
             "graph": "0",
             "extension_group": "predefined graph group",
             "extension": "test extension 1"
           }]
         }
       })"_json);
  ten_test::check_result_is(resp, "137", TEN_STATUS_CODE_OK,
                            "hello world, too");

  delete client;

  ten_thread_join(app_1_thread, -1);
  ten_thread_join(app_2_thread, -1);
}