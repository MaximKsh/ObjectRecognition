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
