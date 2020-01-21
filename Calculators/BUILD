load("@mediapipe_repository//mediapipe/framework/port:build_config.bzl", "mediapipe_cc_proto_library")

cc_library(
    name = "detection_to_text_calculator",
    srcs = ["DetectionToTextCalculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
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
