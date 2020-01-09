#include <iostream>
#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

mediapipe::Status RunGraph() {
    mediapipe::CalculatorGraphConfig config = mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(R"(
    input_stream: "in"
    output_stream: "out"
    node {
      calculator: "HttpTestCalculator"
      input_stream: "in"
      output_stream: "out"
    }
    )");

    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));
    ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller, graph.AddOutputStreamPoller("out"));

    MP_RETURN_IF_ERROR(graph.StartRun({}));

    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            "in",
            mediapipe::MakePacket<std::string>("http://worldtimeapi.org/api/ip.txt").At(mediapipe::Timestamp(0))));

    MP_RETURN_IF_ERROR(graph.CloseInputStream("in"));

    mediapipe::Packet packet;
    while (poller.Next(&packet)) {
        std::cout << packet.Get<std::string>() << std::endl;
    }
    return graph.WaitUntilDone();
}


int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    CHECK(RunGraph().ok());

    return 0;
}
