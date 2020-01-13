package com.kshmax.objectrecognition

import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.mediapipe.framework.Graph
import com.google.mediapipe.framework.PacketCreator
import com.google.mediapipe.framework.PacketGetter
import java.io.BufferedInputStream
import java.io.InputStream
import java.net.HttpURLConnection
import java.net.URL


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        this.setContentView(R.layout.activity_main)
        val outputTv = findViewById<TextView>(R.id.outputTv)

        val graph = Graph()
        assets.open("mobile_binary_graph.binarypb").use {
            val graphBytes = it.readBytes()
            graph.loadBinaryGraph(graphBytes)
        }

        graph.addPacketCallback("out") {
            val res = PacketGetter.getString(it)
            this@MainActivity.runOnUiThread {
                outputTv.text = res
            }
        }
        graph.startRunningGraph()

        val creator = PacketCreator(graph)
        val packet = creator.createString("http://worldtimeapi.org/api/ip.txt")


        graph.addPacketToInputStream("in", packet, 0)



    }

    companion object {
        init {
//            System.loadLibrary("PocoEncodings")
//            System.loadLibrary("PocoFoundation")
//            System.loadLibrary("PocoNet")
//            System.loadLibrary("PocoXML")
//            System.loadLibrary("PocoJSON")
            System.loadLibrary("mediapipe_jni")
        }
    }

}