#!/bin/bash

ADB="/home/maxim/Android/Sdk/platform-tools/adb"

$ADB push ../bazel-bin/Evaluation/Android /data/local/tmp/evaluation
$ADB push data/matrix.txt /data/local/tmp/evaluation_data/matrix.txt
$ADB push data/ssdlite_object_detection.tflite /data/local/tmp/evaluation_data/ssdlite_object_detection.tflite

$ADB shell /data/local/tmp/evaluation /data/local/tmp/evaluation_data/ssdlite_object_detection.tflite /data/local/tmp/evaluation_data/matrix.txt /data/local/tmp/evaluation_data/android

$ADB pull /data/local/tmp/evaluation_data/android-box_encodings.txt data/
$ADB pull /data/local/tmp/evaluation_data/android-class_predictions.txt data/

$ADB shell rm -r /data/local/tmp/evaluation_data /data/local/tmp/evaluation