#include <android/log.h>
#include "mediapipe//framework/packet.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "Calculators/DetectionFilterCalculator.pb.h"

class DetectionFilterCalculator : public mediapipe::CalculatorBase {
public:
    static mediapipe::Status GetContract(mediapipe::CalculatorContract* cc);

    mediapipe::Status Open(mediapipe::CalculatorContext* cc) override;

    mediapipe::Status Process(mediapipe::CalculatorContext* cc) override;

private:
    std::vector<int> pass_ids_;
};

REGISTER_CALCULATOR(DetectionFilterCalculator);

mediapipe::Status DetectionFilterCalculator::GetContract(mediapipe::CalculatorContract *cc) {
    cc->Inputs().Get("", 0).Set<std::vector<mediapipe::Detection>>();
    cc->Outputs().Get("", 0).Set<std::vector<mediapipe::Detection>>();

    return mediapipe::OkStatus();
}

mediapipe::Status DetectionFilterCalculator::Open(mediapipe::CalculatorContext *cc) {
    cc->SetOffset(::mediapipe::TimestampDiff(0));

    const auto& options = cc->Options<objectrecognition::DetectionFilterCalculatorOptions>();
    pass_ids_ = std::vector<int>(options.pass_id().begin(), options.pass_id().end());

    __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%d", pass_ids_.size());

    return ::mediapipe::OkStatus();
}

mediapipe::Status DetectionFilterCalculator::Process(mediapipe::CalculatorContext *cc) {
    const auto& input_detections =
            cc->Inputs().Get("", 0).Get<std::vector<::mediapipe::Detection>>();
    std::vector<::mediapipe::Detection> output_detections;

    for (const auto& input_detection : input_detections) {
        bool next_detection = false;

        for (int pass_id : pass_ids_) {
            for (int label_id : input_detection.label_id()) {
                if (pass_id == label_id) {
                    output_detections.push_back(input_detection);
                    next_detection = true;
                    break;
                }
            }

            if (next_detection) {
                break;
            }
        }
    }

    auto out_packet = mediapipe::MakePacket<std::vector<mediapipe::Detection>>(output_detections)
            .At(cc->InputTimestamp());
    cc->Outputs().Get("", 0).AddPacket(out_packet);

    return mediapipe::OkStatus();
}
