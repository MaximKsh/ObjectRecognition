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
        #"@mediapipe_repository//third_party:androidx_appcompat",
        #"@mediapipe_repository//third_party:androidx_constraint_layout",
        #"@mediapipe_repository//third_party:opencv",
    ],
)

# Copyright 2019 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#licenses(["notice"])  # Apache 2.0
#
#package(default_visibility = ["//visibility:public"])
#
#cc_binary(
#    name = "libmediapipe_jni.so",
#    linkshared = 1,
#    linkstatic = 1,
#    deps = [
#        "@mediapipe_repository//mediapipe/graphs/edge_detection:mobile_calculators",
#        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/framework/jni:mediapipe_framework_jni",
#    ],
#)
#
#cc_library(
#    name = "mediapipe_jni_lib",
#    srcs = [":libmediapipe_jni.so"],
#    alwayslink = 1,
#)
#
#android_library(
#    name = "lib",
#    srcs = glob(["*.java"]),
#    manifest = "AndroidManifest.xml",
#    resource_files = glob(["res/**"]),
#    deps = [
#        ":mediapipe_jni_lib",
#        "@androidx_concurrent_futures//jar",
#        "@com_google_guava_android//jar",
#        "@maven//:androidx_appcompat_appcompat",
#        "@maven//:androidx_core_core",
#        #        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/components:android_camerax_helper",
#        #        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/components:android_components",
#        #        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/framework:android_framework",
#        #        "@mediapipe_repository//mediapipe/java/com/google/mediapipe/glutil",
#        #        "@mediapipe_repository//third_party:androidx_appcompat",
#        #        "@mediapipe_repository//third_party:androidx_constraint_layout",
#        #        "@mediapipe_repository//third_party:opencv",
#    ],
#)
