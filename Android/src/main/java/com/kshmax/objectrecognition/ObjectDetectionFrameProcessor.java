package com.kshmax.objectrecognition;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import com.google.common.base.Preconditions;
import com.google.mediapipe.components.TextureFrameConsumer;
import com.google.mediapipe.components.TextureFrameProcessor;
import com.google.mediapipe.framework.AndroidAssetUtil;
import com.google.mediapipe.framework.AndroidPacketCreator;
import com.google.mediapipe.framework.Graph;
import com.google.mediapipe.framework.GraphService;
import com.google.mediapipe.framework.MediaPipeException;
import com.google.mediapipe.framework.Packet;
import com.google.mediapipe.framework.PacketCallback;
import com.google.mediapipe.framework.PacketGetter;
import com.google.mediapipe.framework.SurfaceOutput;
import com.google.mediapipe.framework.TextureFrame;
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicBoolean;
import com.google.gson.Gson;
import com.kshmax.objectrecognition.proto.ClickLocationProto;

/**
 * A {@link com.google.mediapipe.components.TextureFrameProcessor} that sends video frames through a
 * MediaPipe graph.
 */
public class ObjectDetectionFrameProcessor implements TextureFrameProcessor {
  private static final String TAG = "FrameProcessor";

  private List<TextureFrameConsumer> consumers = new ArrayList<>();
  private Graph mediapipeGraph;
  private AndroidPacketCreator packetCreator;
  private OnWillAddFrameListener addFrameListener;
  private String videoInputStream;
  private String videoInputStreamCpu;
  private String videoOutputStream;
  private SurfaceOutput videoSurfaceOutput;
  private final AtomicBoolean started = new AtomicBoolean(false);
  private boolean hybridPath = false;

  private ConcurrentLinkedQueue<ClickLocationProto.ClickLocation> click_queue = new ConcurrentLinkedQueue<ClickLocationProto.ClickLocation>();
  private Gson gson = new Gson();

  /**
   * Constructor.
   *
   * @param context an Android {@link Context}.
   * @param parentNativeContext a native handle to a GL context. The GL context(s) used by the
   *     calculators in the graph will join the parent context's sharegroup, so that textures
   *     generated by the calculators are available in the parent context, and vice versa.
   * @param inputStream the graph input stream that will receive input video frames.
   * @param outputStream the output stream from which output frames will be produced.
   */
  public ObjectDetectionFrameProcessor(
          Context context,
          long parentNativeContext,
          Graph graph,
          String inputStream,
          String outputStream) {
    mediapipeGraph = graph;
    videoInputStream = inputStream;
    videoOutputStream = outputStream;

    try {

      packetCreator = new AndroidPacketCreator(mediapipeGraph);
      mediapipeGraph.addPacketCallback(
              videoOutputStream,
              new PacketCallback() {
                @Override
                public void process(Packet packet) {
                  List<TextureFrameConsumer> currentConsumers;
                  synchronized (this) {
                    currentConsumers = consumers;
                  }
                  for (TextureFrameConsumer consumer : currentConsumers) {
                    TextureFrame frame = PacketGetter.getTextureFrame(packet);
                    if (Log.isLoggable(TAG, Log.VERBOSE)) {
                      Log.v(
                              TAG,
                              String.format(
                                      "Output tex: %d width: %d height: %d to consumer %h",
                                      frame.getTextureName(), frame.getWidth(), frame.getHeight(), consumer));
                    }
                    consumer.onNewFrame(frame);
                  }
                }
              });

      mediapipeGraph.setParentGlContext(parentNativeContext);
    } catch (MediaPipeException e) {
      Log.e(TAG, "Mediapipe error: ", e);
    }

    videoSurfaceOutput = mediapipeGraph.addSurfaceOutput(videoOutputStream);
  }


  /**
   * Interface to be used so that this class can receive a callback when onNewFrame has determined
   * it will process an input frame. Can be used to feed packets to accessory streams.
   */
  public interface OnWillAddFrameListener {
    void onWillAddFrame(long timestamp);
  }

  public synchronized <T> void setServiceObject(GraphService<T> service, T object) {
    mediapipeGraph.setServiceObject(service, object);
  }

  public void setInputSidePackets(Map<String, Packet> inputSidePackets) {
    Preconditions.checkState(
            !started.get(), "setInputSidePackets must be called before the graph is started");
    mediapipeGraph.setInputSidePackets(inputSidePackets);
  }

  @Override
  public void setConsumer(TextureFrameConsumer listener) {
    synchronized (this) {
      consumers = Arrays.asList(listener);
    }
  }

  public void setVideoInputStreamCpu(String inputStream) {
    videoInputStreamCpu = inputStream;
  }

  public void setHybridPath() {
    hybridPath = true;
  }

  /** Adds a callback to the graph to process packets from the specified output stream. */
  public void addPacketCallback(String outputStream, PacketCallback callback) {
    mediapipeGraph.addPacketCallback(outputStream, callback);
  }

  public void addConsumer(TextureFrameConsumer listener) {
    synchronized (this) {
      List<TextureFrameConsumer> newConsumers = new ArrayList<>(consumers);
      newConsumers.add(listener);
      consumers = newConsumers;
    }
  }

  public boolean removeConsumer(TextureFrameConsumer listener) {
    boolean existed;
    synchronized (this) {
      List<TextureFrameConsumer> newConsumers = new ArrayList<>(consumers);
      existed = newConsumers.remove(listener);
      consumers = newConsumers;
    }
    return existed;
  }

  /** Gets the {@link Graph} used to run the graph. */
  public Graph getGraph() {
    return mediapipeGraph;
  }

  public AndroidPacketCreator getPacketCreator() {
    return packetCreator;
  }

  /** Gets the {@link SurfaceOutput} connected to the video output stream. */
  public SurfaceOutput getVideoSurfaceOutput() {
    return videoSurfaceOutput;
  }

  /** Closes and cleans up the graph. */
  public void close() {
    if (started.get()) {
      try {
        mediapipeGraph.closeAllPacketSources();
        mediapipeGraph.waitUntilGraphDone();
      } catch (MediaPipeException e) {
        Log.e(TAG, "Mediapipe error: ", e);
      }
      try {
        mediapipeGraph.tearDown();
      } catch (MediaPipeException e) {
        Log.e(TAG, "Mediapipe error: ", e);
      }
    }
  }

  /**
   * Initializes the graph in advance of receiving frames.
   *
   * <p>Normally the graph is initialized when the first frame arrives. You can optionally call this
   * method to initialize it ahead of time.
   * @throws MediaPipeException for any error status.
   */
  public void preheat() {
    if (!started.getAndSet(true)) {
      startGraph();
    }
  }

  public void setOnWillAddFrameListener(OnWillAddFrameListener addFrameListener) {
    this.addFrameListener = addFrameListener;
  }

  /**
   * Returns true if the MediaPipe graph can accept one more input frame.
   * @throws MediaPipeException for any error status.
   */
  private boolean maybeAcceptNewFrame() {
    if (!started.getAndSet(true)) {
      startGraph();
    }
    return true;
  }

  @Override
  public void onNewFrame(final TextureFrame frame) {
    if (Log.isLoggable(TAG, Log.VERBOSE)) {
      Log.v(
              TAG,
              String.format(
                      "Input tex: %d width: %d height: %d",
                      frame.getTextureName(), frame.getWidth(), frame.getHeight()));
    }

    if (!maybeAcceptNewFrame()) {
      frame.release();
      return;
    }

    if (addFrameListener != null) {
      addFrameListener.onWillAddFrame(frame.getTimestamp());
    }

    ClickLocationProto.ClickLocation click = click_queue.poll();
    if (click == null) {
      click = ClickLocationProto.ClickLocation.newBuilder().setX(-1).setY(-1).build();
    }

    Packet imagePacket = packetCreator.createGpuBuffer(frame);
    Packet clickPacket = packetCreator.createSerializedProto(click);

    try {
      // addConsumablePacketToInputStream allows the graph to take exclusive ownership of the
      // packet, which may allow for more memory optimizations.
      mediapipeGraph.addConsumablePacketToInputStream(videoInputStream, imagePacket, frame.getTimestamp());
      mediapipeGraph.addConsumablePacketToInputStream("click", clickPacket, frame.getTimestamp());

    } catch (MediaPipeException e) {
      Log.e(TAG, "Mediapipe error: ", e);
    }
    imagePacket.release();
    clickPacket.release();

  }

  /**
   * Accepts a Bitmap to be sent to main input stream at the given timestamp.
   *
   * <p>Note: This requires a graph that takes an ImageFrame instead of a mediapipe::GpuBuffer. An
   * instance of FrameProcessor should only ever use this or the other variant for onNewFrame().
   */
  public void onNewFrame(final Bitmap bitmap, long timestamp) {
    if (!maybeAcceptNewFrame()) {
      return;
    }

    if (!hybridPath && addFrameListener != null) {
      addFrameListener.onWillAddFrame(timestamp);
    }

    Packet packet = getPacketCreator().createRgbImageFrame(bitmap);

    try {
      // addConsumablePacketToInputStream allows the graph to take exclusive ownership of the
      // packet, which may allow for more memory optimizations.
      mediapipeGraph.addConsumablePacketToInputStream(videoInputStreamCpu, packet, timestamp);
    } catch (MediaPipeException e) {
      Log.e(TAG, "Mediapipe error: ", e);
    }
    packet.release();
  }

  public void waitUntilIdle() {
    try {
      mediapipeGraph.waitUntilGraphIdle();
    } catch (MediaPipeException e) {
      Log.e(TAG, "Mediapipe error: ", e);
    }
  }

  public void addClick(float x, float y) {
    ClickLocationProto.ClickLocation loc = ClickLocationProto.ClickLocation
            .newBuilder()
            .setX(x)
            .setY(y)
            .build();

    click_queue.add(loc);
  }

  /**
   * Starts running the MediaPipe graph.
   * @throws MediaPipeException for any error status.
   */
  private void startGraph() {
    mediapipeGraph.startRunningGraph();
  }
}
