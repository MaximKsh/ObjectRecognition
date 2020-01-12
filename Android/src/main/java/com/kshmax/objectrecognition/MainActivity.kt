package com.kshmax.objectrecognition

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.google.mediapipe.components.CameraHelper
import com.google.mediapipe.components.CameraXPreviewHelper
import com.google.mediapipe.components.ExternalTextureConverter
import com.google.mediapipe.components.FrameProcessor
import com.google.mediapipe.components.PermissionHelper
import com.google.mediapipe.framework.AndroidAssetUtil
import com.google.mediapipe.glutil.EglManager
import com.google.mediapipe.framework.Graph


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        this.setContentView(R.layout.activity_main)

        val graph = Graph();

    }

    companion object {
        init {
            System.loadLibrary("mediapipe_jni")
        }
    }

}