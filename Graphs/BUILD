package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mobile_calculators",
    deps = [
        "//Calculators:http_test_calculator",
    ],
)

cc_library(
    name = "object_recognition_mobile_calculators",
    deps = [
        "//Calculators:boundary_box_crop_calculator",
        "//Calculators:detection_filter_calculator",
        "//Calculators:detection_to_text_calculator",
        "//Calculators:echo_calculator",
        "//Calculators:label_provider_calculator",
        "@mediapipe_repository//mediapipe/calculators/core:flow_limiter_calculator",
        "@mediapipe_repository//mediapipe/calculators/image:image_transformation_calculator",
        "@mediapipe_repository//mediapipe/calculators/tflite:ssd_anchors_calculator",
        "@mediapipe_repository//mediapipe/calculators/tflite:tflite_converter_calculator",
        "@mediapipe_repository//mediapipe/calculators/tflite:tflite_inference_calculator",
        "@mediapipe_repository//mediapipe/calculators/tflite:tflite_tensors_to_detections_calculator",
        "@mediapipe_repository//mediapipe/calculators/tflite:tflite_tensors_to_floats_calculator",
        "@mediapipe_repository//mediapipe/calculators/util:annotation_overlay_calculator",
        "@mediapipe_repository//mediapipe/calculators/util:detection_letterbox_removal_calculator",
        "@mediapipe_repository//mediapipe/calculators/util:detections_to_render_data_calculator",
        "@mediapipe_repository//mediapipe/calculators/util:non_max_suppression_calculator",
        "@mediapipe_repository//mediapipe/gpu:gpu_buffer_to_image_frame_calculator",
        "@mediapipe_repository//mediapipe/gpu:image_frame_to_gpu_buffer_calculator",
    ],
)

load("//mediapipe/framework/tool:mediapipe_graph.bzl", "mediapipe_binary_graph")

mediapipe_binary_graph(
    name = "mobile_cpu_binary_graph",
    graph = "HttpTestGraph.pbtxt",
    output_name = "mobile_cpu.binarypb",
    deps = [":mobile_calculators"],
)

mediapipe_binary_graph(
    name = "object_recognition_mobile_binary_graph",
    graph = "ObjectRecognitionGraph.pbtxt",
    output_name = "object_recognition_mobile.binarypb",
    deps = [":object_recognition_mobile_calculators"],
)
