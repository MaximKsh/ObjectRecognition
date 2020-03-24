package com.kshmax.objectrecognition

data class ClickLocation(
        val x: Float,
        val y: Float
)

data class CarDescriptor(
        val car: FloatArray
)

data class RecognitionResult(
        val label: String
)