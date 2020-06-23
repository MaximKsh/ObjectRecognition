package com.kshmax.objectrecognition

import android.app.Activity
import android.app.AlertDialog
import android.content.Context
import android.graphics.SurfaceTexture
import android.os.AsyncTask
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Size
import android.view.*
import android.widget.EditText
import android.widget.LinearLayout
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
import java.lang.Exception


class MainActivity : AppCompatActivity() {
    class TestConnectionRequestTask(
        private val url: String,
        private val context: Context) : AsyncTask<Void, Void, String>() {

        override fun doInBackground(vararg params: Void): String {
            val client = OkHttpClient()
            val url = "${url}/test"
            val request = Request.Builder()
                    .url(url)
                    .get()
                    .build()

            return try {
                client.newCall(request).execute().use {
                    response -> response.body!!.string()
                }
            } catch (e: Exception) {
                e.message ?: "Backend service is not available"
            }
        }

        override fun onPostExecute(result: String) {
            val alertDialog = AlertDialog.Builder(context)
            alertDialog.setTitle("Test connection")
            alertDialog.setMessage(result)

            alertDialog.setPositiveButton("Got it") { dialog, _ ->
                dialog.cancel()
            }
            alertDialog.show()
        }

    }

    data class RecognitionRequestTaskResult(
            val result: RecognitionResult? = null,
            val error: String? = null
    )

    class RecognitionRequestTask(
            private val url: String,
            private val tv: TextView) : AsyncTask<CarDescriptor, Void, RecognitionRequestTaskResult>() {
        private val JSON = "application/json; charset=utf-8".toMediaType()

        override fun doInBackground(vararg carDescriptor: CarDescriptor): RecognitionRequestTaskResult {
            val gson = Gson()
            val client = OkHttpClient()
            val body = gson.toJson(carDescriptor[0]).toRequestBody(JSON)
            val url = "${url}/recognize"
            val request = Request.Builder()
                    .url(url)
                    .post(body)
                    .build()

            return try {
                val resultJson = client.newCall(request).execute().use { response -> response.body!!.string()
                }

                RecognitionRequestTaskResult(gson.fromJson(resultJson, RecognitionResult::class.java))
            } catch (e: Exception) {
                RecognitionRequestTaskResult(null, e.message ?: "Backend service is not available")
            }
        }

        override fun onPostExecute(result: RecognitionRequestTaskResult) {
            tv.text = result.result?.label ?: result.error ?: "Unknown error"
        }
    }

    private val URL_KEY = "URL"
    private val DEFAULT_URL = "http://92.53.78.235:5000"
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

    private var lastTime: Long = 0
    private var lastTapTime: Long = 0

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

        val desc_time = this@MainActivity.findViewById<TextView>(R.id.descriptor_time_textview)
        objectDetectionFrameProcessor.addPacketCallback("car_vector") {
            Handler(Looper.getMainLooper()).post {
                desc_time.text = (System.currentTimeMillis() - lastTapTime).toString()
            }

            val car = PacketGetter.getFloat32Vector(it)
            val descriptor = CarDescriptor(car)
            RecognitionRequestTask(getUrl(), tv).execute(descriptor)
        }

        val fps = this@MainActivity.findViewById<TextView>(R.id.fps_textview)
        objectDetectionFrameProcessor.addPacketCallback("output_video") {
            val time = System.currentTimeMillis()
            val currentFps = (1000 / (time - lastTime)).toInt().toString()
            lastTime = time

            Handler(Looper.getMainLooper()).post {
                fps.text = currentFps
            }
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

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        val inflater: MenuInflater = menuInflater
        inflater.inflate(R.menu.main_menu, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.change_url -> {
                changeUrlDialog()
                true
            }
            R.id.test_connection -> {
                testConnection()
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
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
                lastTapTime = System.currentTimeMillis()
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

    private fun testConnection() {
        TestConnectionRequestTask(getUrl(), this).execute()
    }

    private fun changeUrlDialog() {
        val alertDialog = AlertDialog.Builder(this@MainActivity)
        alertDialog.setTitle("Set URL")
        alertDialog.setMessage("Specify backend URL in format \"http://domain:port\" (without trailing slash)")

        val input = EditText(this@MainActivity)
        val lp = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT)
        input.layoutParams = lp
        alertDialog.setView(input)
        input.setText(getUrl())

        alertDialog.setPositiveButton("Confirm") { dialog, _ ->
            setUrl(input.text.toString())
            dialog.cancel()
        }

        alertDialog.setNegativeButton("Cancel") { dialog, _ -> dialog.cancel() }

        alertDialog.show()
    }

    private fun getUrl() : String {
        val sharedPref = this.getPreferences(Context.MODE_PRIVATE) ?: return DEFAULT_URL
        return sharedPref.getString(URL_KEY, DEFAULT_URL) ?: DEFAULT_URL
    }

    private fun setUrl(url: String) {
        val sharedPref = this.getPreferences(Context.MODE_PRIVATE) ?: return
        with (sharedPref.edit()) {
            putString(URL_KEY, url)
            commit()
        }
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