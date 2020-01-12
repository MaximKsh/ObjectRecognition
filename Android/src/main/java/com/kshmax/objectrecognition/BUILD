load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_android_library")

cc_binary(
    name = "libmediapipe_jni.so",
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "@mediapipe_repository//mediapipe/graphs/edge_detection:mobile_calculators",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/framework/jni:mediapipe_framework_jni",
    ],
)

cc_library(
    name = "mediapipe_jni_lib",
    srcs = [":libmediapipe_jni.so"],
    alwayslink = 1,
)

kt_android_library(
    name = "lib",
    srcs = glob(["*.kt"]),
    custom_package = "com.kshmax.objectrecognition",
    manifest = "AndroidManifest.xml",
    resource_files = glob(["res/**"]),
    visibility = ["//visibility:public"],
    exports = [
        ":mediapipe_jni_lib",
        "@mediapipe_repository//third_party:opencv",
    ],
    deps = [
        "@androidx_concurrent_futures//jar",
        "@com_google_guava_android//jar",
        "@maven//:androidx_appcompat_appcompat",
        "@maven//:androidx_core_core",
        "@maven//:androidx_core_core_ktx",
        "@maven//:androidx_drawerlayout_drawerlayout",
        "@maven//:androidx_fragment_fragment",
        "@maven//:androidx_lifecycle_lifecycle_common",
        "@maven//:androidx_lifecycle_lifecycle_viewmodel",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/components:android_camerax_helper",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/components:android_components",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/framework:android_framework",
        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/glutil",
        "@mediapipe_repository//third_party:androidx_constraint_layout",
    ],
)
