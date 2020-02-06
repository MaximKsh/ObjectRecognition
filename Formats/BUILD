load("@rules_java//java:defs.bzl", "java_binary", "java_lite_proto_library", "java_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

proto_library(
    name = "clicklocation_proto",
    srcs = ["ClickLocation.proto"],
)

java_lite_proto_library(
    name = "clicklocation_java_lite_proto",
    deps = [":clicklocation_proto"],
)
