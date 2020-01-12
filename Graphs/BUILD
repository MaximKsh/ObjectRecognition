package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mobile_calculators",
    deps = [
        "//Calculators:echo_calculator",
    ],
)

load("//mediapipe/framework/tool:mediapipe_graph.bzl", "mediapipe_binary_graph")

mediapipe_binary_graph(
    name = "mobile_cpu_binary_graph",
    graph = "HttpTestGraph.pbtxt",
    output_name = "mobile_cpu.binarypb",
    deps = [":mobile_calculators"],
)
