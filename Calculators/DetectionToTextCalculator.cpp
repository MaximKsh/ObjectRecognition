#include <android/log.h>
#include "mediapipe//framework/packet.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_format.pb.h"

// #include "mediapipe/framework/port/opencv_core_inc.h"

class DetectionToTextCalculator : public ::mediapipe::CalculatorBase {
public:
    static ::mediapipe::Status GetContract(::mediapipe::CalculatorContract* cc);

    ::mediapipe::Status Open(::mediapipe::CalculatorContext* cc) override;

    ::mediapipe::Status Process(::mediapipe::CalculatorContext* cc) override;

private:
    std::unordered_map<int, std::string> label_map_;
/*
    mediapipe::Status CreateRenderTargetGpu(
            mediapipe::CalculatorContext* cc,
            std::unique_ptr<cv::Mat>& image_mat);
            */
};

REGISTER_CALCULATOR(DetectionToTextCalculator);

::mediapipe::Status DetectionToTextCalculator::GetContract(::mediapipe::CalculatorContract *cc) {
    cc->Inputs().Get("", 0).Set<std::vector<::mediapipe::Detection>>();
    cc->Outputs().Get("", 0).Set<std::vector<::mediapipe::Detection>>();

    //cc->Inputs().Tag("INPUT_FRAME_GPU").Set<mediapipe::ImageFrame>();

    //cc->InputSidePackets().Index(0).Set<std::string>();
    //cc->Inputs().Tag("LABELS_IN").Set<std::string>();
    if (cc->Inputs().HasTag("SECOND")) {
        cc->Inputs().Tag("SECOND").SetAny();
    }
    cc->Outputs().Tag("LABELS_OUT").Set<std::vector<int32_t>>();

    return ::mediapipe::OkStatus();
}

::mediapipe::Status DetectionToTextCalculator::Open(::mediapipe::CalculatorContext *cc) {
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

::mediapipe::Status DetectionToTextCalculator::Process(::mediapipe::CalculatorContext *cc) {
    std::vector<::mediapipe::Detection> output_detections;
    std::vector<int32_t> unknown_labels;

    auto& string_packet = cc->Inputs().Get("SECOND", 0);
    if (!string_packet.IsEmpty()) {
        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", string_packet.Get<std::string>().c_str());
    }

//    auto& frames_packet = cc->Inputs().Get("INPUT_FRAME_GPU", 0);
//
//    if (!frames_packet.IsEmpty()) {
//        const mediapipe::ImageFrame& image_frame = frames_packet.Get<mediapipe::ImageFrame>();
//        // std::unique_ptr<uint8> buffer = std::unique_ptr<uint8>(new uint8[image_frame.PixelDataSize()]);
//        // image_frame.CopyToBuffer(buffer.get(), image_frame.PixelDataSize());
//
//        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "WxH %dx%d", image_frame.Width(), image_frame.Height());
//
//        const uint8* pixel_data = image_frame.PixelData();
//        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", std::to_string(*pixel_data).c_str());
//        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", std::to_string(image_frame.PixelDataSizeStoredContiguously()).c_str());
//
//
//       // mediapipe::GlTextureBuffer texture_buffer = gpu_buffer.GetGlTextureBufferSharedPtr().get();
//
//        //std::unique_ptr<cv::Mat> image_mat;
//        //MP_RETURN_IF_ERROR(CreateRenderTargetGpu(cc, image_mat));
//
//    }


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
/*
mediapipe::Status DetectionToTextCalculator::CreateRenderTargetGpu(
        mediapipe::CalculatorContext* cc,
        std::unique_ptr<cv::Mat>& image_mat) {

    if (image_frame_available_) {
        const auto& input_frame =
                cc->Inputs().Tag(kInputFrameTagGpu).Get<mediapipe::GpuBuffer>();

        const mediapipe::ImageFormat::Format format =
                mediapipe::ImageFormatForGpuBufferFormat(input_frame.format());
        if (format != mediapipe::ImageFormat::SRGBA &&
            format != mediapipe::ImageFormat::SRGB)
            RET_CHECK_FAIL() << "Unsupported GPU input format: " << format;

        image_mat = absl::make_unique<cv::Mat>(
                height_, width_, CV_8UC3,
                cv::Scalar(kAnnotationBackgroundColor[0], kAnnotationBackgroundColor[1],
                           kAnnotationBackgroundColor[2]));
    } else {
        image_mat = absl::make_unique<cv::Mat>(
                options_.canvas_height_px(), options_.canvas_width_px(), CV_8UC3,
                cv::Scalar(options_.canvas_color().r(), options_.canvas_color().g(),
                           options_.canvas_color().b()));
    }

    return ::mediapipe::OkStatus();
}*/
