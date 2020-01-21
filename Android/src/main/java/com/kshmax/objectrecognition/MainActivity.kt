package com.kshmax.objectrecognition

import android.graphics.SurfaceTexture
import android.os.Bundle
import android.util.Size
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.mediapipe.components.CameraHelper.CameraFacing
import com.google.mediapipe.components.CameraHelper.OnCameraStartedListener
import com.google.mediapipe.components.CameraXPreviewHelper
import com.google.mediapipe.components.ExternalTextureConverter
import com.google.mediapipe.components.FrameProcessor
import com.google.mediapipe.components.PermissionHelper
import com.google.mediapipe.framework.AndroidAssetUtil
import com.google.mediapipe.framework.PacketCreator
import com.google.mediapipe.framework.PacketGetter
import com.google.mediapipe.glutil.EglManager


class MainActivity : AppCompatActivity() {

    private val BINARY_GRAPH_NAME = "mobile_binary_graph.binarypb"
    private val INPUT_VIDEO_STREAM_NAME = "input_video"
    private val OUTPUT_VIDEO_STREAM_NAME = "output_video"
    private val CAMERA_FACING = CameraFacing.BACK

    private val FLIP_FRAMES_VERTICALLY = true
    private val LABELS = """
        person
        bicycle
        car
        motorcycle
        airplane
        bus
        train
        truck
        boat
        traffic light
        fire hydrant
        ???
        stop sign
        parking meter
        bench
        bird
        cat
        dog
        horse
        sheep
        cow
        elephant
        bear
        zebra
        giraffe
        ???
        backpack
        umbrella
        ???
        ???
        handbag
        tie
        suitcase
        frisbee
        skis
        snowboard
        sports ball
        kite
        baseball bat
        baseball glove
        skateboard
        surfboard
        tennis racket
        bottle
        ???
        wine glass
        cup
        fork
        knife
        spoon
        bowl
        banana
        apple
        sandwich
        orange
        broccoli
        carrot
        hot dog
        pizza
        donut
        cake
        chair
        couch
        potted plant
        bed
        ???
        dining table
        ???
        ???
        toilet
        ???
        tv
        laptop
        mouse
        remote
        keyboard
        cell phone
        microwave
        oven
        toaster
        sink
        refrigerator
        ???
        book
        clock
        vase
        scissors
        teddy bear
        hair drier
        toothbrush
    """.trimIndent()

    // {@link SurfaceTexture} where the camera-preview frames can be accessed.
    private var previewFrameTexture: SurfaceTexture? = null
    // {@link SurfaceView} that displays the camera-preview frames processed by a MediaPipe graph.
    private var previewDisplayView: SurfaceView? = null

    // Creates and manages an {@link EGLContext}.
    private var eglManager: EglManager? = null
    // Sends camera-preview frames into a MediaPipe graph for processing, and displays the processed
    // frames onto a {@link Surface}.
    private var processor: FrameProcessor? = null

    // Converts the GL_TEXTURE_EXTERNAL_OES texture from Android camera into a regular texture to be
    // consumed by {@link FrameProcessor} and the underlying MediaPipe graph.
    private var converter: ExternalTextureConverter? = null

    // Handles camera access via the {@link CameraX} Jetpack support library.
    private var cameraHelper: CameraXPreviewHelper? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        this.setContentView(R.layout.activity_main)

        previewDisplayView = SurfaceView(this)
        setupPreviewDisplayView()

        // Initialize asset manager so that MediaPipe native libraries can access the app assets, e.g.,
        // binary graphs.
        // Initialize asset manager so that MediaPipe native libraries can access the app assets, e.g.,
        // binary graphs.
        AndroidAssetUtil.initializeNativeAssetManager(this)

        eglManager = EglManager(null)
        processor = FrameProcessor(
                this,
                eglManager!!.nativeContext,
                BINARY_GRAPH_NAME,
                INPUT_VIDEO_STREAM_NAME,
                OUTPUT_VIDEO_STREAM_NAME)
        processor!!.videoSurfaceOutput.setFlipY(FLIP_FRAMES_VERTICALLY)

        val labels = LABELS.split("\n")
        processor!!.addPacketCallback("labels") {
            val arr = PacketGetter.getInt32Vector(it)
            var answers = ArrayList<String>()
            for (label in arr) {
                val answer = if (label in 0..labels.size) {
                    "$label ${labels[label]}"
                } else {
                    "$label ???"
                }
                answers.add(answer)
            }


            val creator = PacketCreator(processor!!.graph)
            val labels_packet = creator.createString(answers.joinToString("\n"))

            processor!!.graph.addPacketToInputStream("labels_with_values", labels_packet, it.timestamp + 1)

        }

        PermissionHelper.checkAndRequestCameraPermissions(this)

    }

    override fun onResume() {
        super.onResume()
        converter = ExternalTextureConverter(eglManager!!.context)
        converter!!.setFlipY(FLIP_FRAMES_VERTICALLY)
        converter!!.setConsumer(processor)
        if (PermissionHelper.cameraPermissionsGranted(this)) {
            startCamera()
        }
    }

    override fun onPause() {
        super.onPause()
        converter!!.close()
    }

    override fun onRequestPermissionsResult(
            requestCode: Int,
            permissions: Array<String?>,
            grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions!!, grantResults!!)
        PermissionHelper.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }

    private fun setupPreviewDisplayView() {
        previewDisplayView!!.visibility = View.GONE
        val viewGroup = findViewById<ViewGroup>(R.id.preview_display_layout)
        viewGroup.addView(previewDisplayView)
        previewDisplayView
                ?.holder
                ?.addCallback(
                        object : SurfaceHolder.Callback {
                            override fun surfaceCreated(holder: SurfaceHolder) {
                                processor!!.videoSurfaceOutput.setSurface(holder.surface)
                            }

                            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
                                // (Re-)Compute the ideal size of the camera-preview display (the area that the
                                // camera-preview frames get rendered onto, potentially with scaling and rotation)
                                // based on the size of the SurfaceView that contains the display.
                                val viewSize = Size(width, height)
                                val displaySize = cameraHelper!!.computeDisplaySizeFromViewSize(viewSize)
                                // Connect the converter to the camera-preview frames as its input (via
                                // previewFrameTexture), and configure the output width and height as the computed
                                // display size.
                                converter!!.setSurfaceTextureAndAttachToGLContext(
                                        previewFrameTexture, displaySize.width, displaySize.height)
                            }

                            override fun surfaceDestroyed(holder: SurfaceHolder) {
                                processor!!.videoSurfaceOutput.setSurface(null)
                            }
                        })
    }

    private fun startCamera() {
        cameraHelper = CameraXPreviewHelper()
        cameraHelper!!.setOnCameraStartedListener(
                OnCameraStartedListener { surfaceTexture: SurfaceTexture? ->
                    previewFrameTexture = surfaceTexture
                    // Make the display view visible to start showing the preview. This triggers the
                    // SurfaceHolder.Callback added to (the holder of) previewDisplayView.
                    previewDisplayView!!.visibility = View.VISIBLE
                })
        cameraHelper!!.startCamera(this, CAMERA_FACING,  /*surfaceTexture=*/null)
    }

    companion object {
        init {
//            System.loadLibrary("PocoEncodings")
//            System.loadLibrary("PocoFoundation")
//            System.loadLibrary("PocoNet")
//            System.loadLibrary("PocoXML")
//            System.loadLibrary("PocoJSON")
            System.loadLibrary("mediapipe_jni")
            System.loadLibrary("opencv_java3")
        }
    }

}