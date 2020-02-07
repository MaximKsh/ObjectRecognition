package com.kshmax.objectrecognition

data class ClickLocation(
        val x: Float,
        val y: Float
)

data class CroppedImage(
        val width: Int,
        val height: Int,
        val base64_pixels: String
)

data class RecognitionResult(
        val label: String
)