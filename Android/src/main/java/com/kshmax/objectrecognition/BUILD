package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_android_library")

java_lite_proto_library(
    name = "click_location_java_proto",
    deps = ["//Calculators:click_location_proto"],
)

cc_binary(
    name = "libmediapipe_jni.so",
    linkshared = 1,
    deps = [
        "//Graphs:object_recognition_mobile_calculators",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/framework/jni:mediapipe_framework_jni",
    ],
)

cc_library(
    name = "mediapipe_jni_lib",
    srcs = [":libmediapipe_jni.so"],
    alwayslink = 1,
)

# Maps the binary graph to an alias (e.g., the app name) for convenience so that the alias can be
# easily incorporated into the app via, for example,
# MainActivity.BINARY_GRAPH_NAME = "appname.binarypb".
genrule(
    name = "binary_graph",
    srcs = ["//Graphs:object_recognition_mobile_binary_graph"],
    outs = ["mobile_binary_graph.binarypb"],
    cmd = "cp $< $@",
)

kt_android_library(
    name = "lib",
    srcs = glob([
        "*.kt",
        "*.java",
    ]),
    assets = [
        ":binary_graph",
        "//Models:car_vectorizer.tflite",
        "@mediapipe_repository//mediapipe/models:ssdlite_object_detection.tflite",
        "@mediapipe_repository//mediapipe/models:ssdlite_object_detection_labelmap.txt",
    ],
    assets_dir = "",
    custom_package = "com.kshmax.objectrecognition",
    manifest = "AndroidManifest.xml",
    resource_files = glob(["res/**"]),
    visibility = ["//visibility:public"],
    exports = [
        ":mediapipe_jni_lib",
        "//ThirdParty:libc",
        #"//ThirdParty:poco",
        "@mediapipe_repository//third_party:opencv",
    ],
    deps = [
        ":click_location_java_proto",
        "@androidx_concurrent_futures//jar",
        "@com_google_guava_android//jar",
        "@maven//:androidx_appcompat_appcompat",
        "@maven//:androidx_core_core",
        "@maven//:androidx_core_core_ktx",
        "@maven//:androidx_drawerlayout_drawerlayout",
        "@maven//:androidx_fragment_fragment",
        "@maven//:androidx_lifecycle_lifecycle_common",
        "@maven//:androidx_lifecycle_lifecycle_viewmodel",
        "@maven//:com_google_code_gson_gson",
        "@maven//:com_squareup_okhttp3_okhttp",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/components:android_camerax_helper",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/components:android_components",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/framework:android_framework",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/glutil",
        "@mediapipe_repository//third_party:androidx_constraint_layout",
    ],
)

android_binary(
    name = "objectrecognition",
    dexopts = [
        "--force-jumbo",
    ],
    manifest = "AndroidManifest.xml",
    multidex = "native",
    deps = [":lib"],
)
