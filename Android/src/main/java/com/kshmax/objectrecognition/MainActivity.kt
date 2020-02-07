package com.kshmax.objectrecognition

import android.graphics.SurfaceTexture
import android.os.AsyncTask
import android.os.Bundle
import android.util.Size
import android.view.*
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.gson.Gson
import com.google.mediapipe.components.CameraHelper.CameraFacing
import com.google.mediapipe.components.CameraXPreviewHelper
import com.google.mediapipe.components.ExternalTextureConverter
import com.google.mediapipe.components.PermissionHelper
import com.google.mediapipe.framework.AndroidAssetUtil
import com.google.mediapipe.framework.Graph
import com.google.mediapipe.framework.PacketGetter
import com.google.mediapipe.glutil.EglManager
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody


class MainActivity : AppCompatActivity() {
    inner class RecognitionRequestTask() : AsyncTask<String, Void, RecognitionResult>() {
        private val BACKEND_URL = "http://192.168.1.122:8080/"
        private val JSON = "application/json; charset=utf-8".toMediaType()

        override fun doInBackground(vararg croppedCarJson: String): RecognitionResult {
            val gson = Gson()
            val client = OkHttpClient()
            val body = croppedCarJson[0].toRequestBody(JSON)
            val request = Request.Builder()
                    .url(BACKEND_URL + "recognize")
                    .post(body)
                    .build()

            val resultJson = client.newCall(request).execute().use {
                response -> response.body!!.string()
            }

            return gson.fromJson(resultJson, RecognitionResult::class.java)

        }

        override fun onPostExecute(result: RecognitionResult) {
            val tv = this@MainActivity.findViewById<TextView>(R.id.label_text_view)
            tv.text = result.label
        }
    }

    private val BINARY_GRAPH_NAME = "mobile_binary_graph.binarypb"
    private val INPUT_VIDEO_STREAM_NAME = "input_video"
    private val OUTPUT_VIDEO_STREAM_NAME = "output_video"
    private val CAMERA_FACING = CameraFacing.BACK

    private val FLIP_FRAMES_VERTICALLY = true

    // {@link SurfaceTexture} where the camera-preview frames can be accessed.
    private var previewFrameTexture: SurfaceTexture? = null
    // {@link SurfaceView} that displays the camera-preview frames processed by a MediaPipe graph.
    private var previewDisplayView: SurfaceView? = null

    // Creates and manages an {@link EGLContext}.
    private var eglManager: EglManager? = null
    // Sends camera-preview frames into a MediaPipe graph for processing, and displays the processed
    // frames onto a {@link Surface}.
    private var processor: ObjectDetectionFrameProcessor? = null

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

        val graph = Graph()
        assets.open(BINARY_GRAPH_NAME).use {
            val graphBytes = it.readBytes()
            graph.loadBinaryGraph(graphBytes)
        }

        eglManager = EglManager(null)
        val objectDetectionFrameProcessor = ObjectDetectionFrameProcessor(
                this,
                eglManager!!.nativeContext,
                graph,
                INPUT_VIDEO_STREAM_NAME,
                OUTPUT_VIDEO_STREAM_NAME)
        objectDetectionFrameProcessor.videoSurfaceOutput.setFlipY(FLIP_FRAMES_VERTICALLY)

        val tv = this@MainActivity.findViewById<TextView>(R.id.label_text_view)
        tv.setOnClickListener {
            (it as TextView).text = ""
        }

        objectDetectionFrameProcessor.addPacketCallback("cropped_car") {
            // This is CroppedImage into JSON. It can be deserialized but it is redundant
            val croppedImageJson = PacketGetter.getString(it)
            RecognitionRequestTask().execute(croppedImageJson)
        }

        processor = objectDetectionFrameProcessor
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
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        PermissionHelper.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }

    private fun setupPreviewDisplayView() {
        val displayView = previewDisplayView!!
        displayView.setOnTouchListener { view, event ->
            if (event.action == MotionEvent.ACTION_DOWN) {
                val relativeX = event.x / view.width
                val relativeY = event.y / view.height
                this@MainActivity.processor!!.addClick(relativeX, relativeY)
            }

            true
        }

        displayView.visibility = View.GONE
        val viewGroup = findViewById<ViewGroup>(R.id.preview_display_layout)
        viewGroup.addView(displayView)
        displayView
                .holder
                .addCallback(
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
        cameraHelper!!.setOnCameraStartedListener { surfaceTexture: SurfaceTexture? ->
            previewFrameTexture = surfaceTexture
            // Make the display view visible to start showing the preview. This triggers the
            // SurfaceHolder.Callback added to (the holder of) previewDisplayView.
            previewDisplayView!!.visibility = View.VISIBLE
        }
        cameraHelper!!.startCamera(this, CAMERA_FACING, null)
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