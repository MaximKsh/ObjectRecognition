load("@mediapipe_repository//mediapipe/framework/port:build_config.bzl", "mediapipe_cc_proto_library")

package(default_visibility = ["//visibility:public"])

proto_library(
    name = "detection_filter_calculator_proto",
    srcs = ["DetectionFilterCalculator.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "@mediapipe_repository//mediapipe/framework:calculator_proto",
    ],
)

mediapipe_cc_proto_library(
    name = "detection_filter_calculator_cc_proto",
    srcs = ["DetectionFilterCalculator.proto"],
    cc_deps = [
        "@mediapipe_repository//mediapipe/framework:calculator_cc_proto",
    ],
    visibility = ["//visibility:public"],
    deps = [":detection_filter_calculator_proto"],
)

cc_library(
    name = "detection_filter_calculator",
    srcs = ["DetectionFilterCalculator.cpp"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":detection_filter_calculator_cc_proto",
        "@mediapipe_repository//mediapipe/framework:calculator_framework",
        "@mediapipe_repository//mediapipe/framework:calculator_options_cc_proto",
        "@mediapipe_repository//mediapipe/framework/formats:detection_cc_proto",
        "@mediapipe_repository//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)

cc_library(
    name = "detection_to_text_calculator",
    srcs = ["DetectionToTextCalculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//ThirdParty:poco",
        "@mediapipe_repository//mediapipe/framework:calculator_framework",
        "@mediapipe_repository//mediapipe/framework:packet",
        "@mediapipe_repository//mediapipe/framework/formats:detection_cc_proto",
        "@mediapipe_repository//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)

cc_library(
    name = "echo_calculator",
    srcs = ["EchoCalculator.cpp"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "@mediapipe_repository//mediapipe/framework:calculator_framework",
        "@mediapipe_repository//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)

cc_library(
    name = "http_test_calculator",
    srcs = ["HttpTestCalculator.cpp"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//ThirdParty:poco",
        "@mediapipe_repository//mediapipe/framework:calculator_framework",
        "@mediapipe_repository//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)

cc_library(
    name = "label_provider_calculator",
    srcs = ["LabelProviderCalculator.cpp"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "@mediapipe_repository//mediapipe/framework:calculator_framework",
        "@mediapipe_repository//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)
