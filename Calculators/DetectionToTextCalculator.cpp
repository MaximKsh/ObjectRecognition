#include <android/log.h>
#include "mediapipe//framework/packet.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/detection.pb.h"

class DetectionToTextCalculator : public ::mediapipe::CalculatorBase {
public:
    static ::mediapipe::Status GetContract(::mediapipe::CalculatorContract* cc) {
        cc->Inputs().Get("", 0).Set<std::vector<::mediapipe::Detection>>();
        cc->Outputs().Get("", 0).Set<std::vector<::mediapipe::Detection>>();

        //cc->InputSidePackets().Index(0).Set<std::string>();
        //cc->Inputs().Tag("LABELS_IN").Set<std::string>();
        cc->Outputs().Tag("LABELS_OUT").Set<std::vector<int32_t>>();

        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status Open(::mediapipe::CalculatorContext* cc) override {
        cc->SetOffset(::mediapipe::TimestampDiff(0));


        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", "aaa");
//        std::istringstream stream(label_map_string);
//        std::string line;
        /*int i = 0;
        while (std::getline(stream, line)) {
            label_map_[i++] = line;
            __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", line.c_str());
        }*/
        return ::mediapipe::OkStatus();
    }


    ::mediapipe::Status Process(::mediapipe::CalculatorContext* cc) override {
        std::vector<::mediapipe::Detection> output_detections;
        std::vector<int32_t> unknown_labels;

        for (const auto& input_detection : cc->Inputs().Get("", 0).Get<std::vector<::mediapipe::Detection>>()) {
            output_detections.push_back(input_detection);
            ::mediapipe::Detection& output_detection = output_detections.back();

            bool has_text_label = false;
            for (const int32 label_id : output_detection.label_id()) {
                if (label_map_.find(label_id) != label_map_.end()) {
                    output_detection.add_label(label_map_[label_id]);
                    has_text_label = true;
                } else {
                    unknown_labels.push_back(label_id);
                }
            }

            // Remove label_id field if text labels exist.
            if (has_text_label) {
                output_detection.clear_label_id();
            }
        }
        auto out_packet = ::mediapipe::MakePacket<std::vector<::mediapipe::Detection>>(output_detections)
                .At(cc->InputTimestamp());
        cc->Outputs().Get("", 0).AddPacket(out_packet);

        if (!unknown_labels.empty()) {
            auto unknown_labels_packet = ::mediapipe::MakePacket<std::vector<int32_t>>(unknown_labels)
                    .At(cc->InputTimestamp());
            cc->Outputs().Tag("LABELS_OUT").AddPacket(unknown_labels_packet);
        }

        return ::mediapipe::OkStatus();
    }

private:
    std::unordered_map<int, std::string> label_map_;
};

REGISTER_CALCULATOR(DetectionToTextCalculator);
