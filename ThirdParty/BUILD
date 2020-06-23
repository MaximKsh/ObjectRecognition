package(default_visibility = ["//visibility:public"])

#cc_library(
#    name = "poco",
#    visibility = ["//visibility:public"],
#    deps = select({
#        "//:android": [
#            "@poco_android//:poco_armeabi-v7a",
#        ],
#        "//:android_arm": [
#            "@poco_android//:poco_armeabi-v7a",
#        ],
#        "//:android_arm64": [
#            "@poco_android//:poco_arm64-v8a",
#        ],
#        "//conditions:default": [
#            "@poco_linux//:poco_linux",
#        ],
#    }),
#)

cc_library(
    name = "libc",
    visibility = ["//visibility:public"],
    deps = select({
        "//:android_arm64": [
            "@libc_android//:libc_android_arm64",
        ],
        "//conditions:default": [
        ],
    }),
)
