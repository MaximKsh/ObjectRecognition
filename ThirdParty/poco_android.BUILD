

cc_library(
    name = "poco_armeabi-v7a",
    srcs = [
        "lib/Android/armeabi-v7a/libPocoFoundation.a",
        "lib/Android/armeabi-v7a/libPocoNet.a",
    ],
    hdrs = glob([
        "Net/include/Poco/Net/*.h",
        "Foundation/include/Poco/*.h",
    ]),
    includes = [
        "Net/include",
        "Foundation/include",
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)

cc_library(
    name = "poco_arm64-v8a",
    srcs = [
        "lib/Android/arm64-v8a/libPocoEncodings.so",
        "lib/Android/arm64-v8a/libPocoFoundation.so",
        "lib/Android/arm64-v8a/libPocoJSON.so",
        "lib/Android/arm64-v8a/libPocoNet.so",
        "lib/Android/arm64-v8a/libPocoXML.so",
    ],
    hdrs = glob([
        "Encodings/include/Poco/*.h",
        "Foundation/include/Poco/*.h",
        "JSON/include/Poco/JSON/*.h",
        "Net/include/Poco/Net/*.h",
        "XML/include/Poco/XML/*.h",
    ]),
    includes = [
        "Encodings/include",
        "Foundation/include",
        "JSON/include",
        "Net/include",
        "XML/include",
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)
