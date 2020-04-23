#include <android/log.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include "mediapipe//framework/packet.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "Utils/Base64.h"
#include "Calculators/ClickLocation.pb.h"

class BoundaryBoxCropCalculator : public mediapipe::CalculatorBase {
public:
    static mediapipe::Status GetContract(mediapipe::CalculatorContract* cc);

    mediapipe::Status Open(mediapipe::CalculatorContext* cc) override;

    mediapipe::Status Process(mediapipe::CalculatorContext* cc) override;

private:
    static absl::optional<mediapipe::Detection> FindOverlappedDetection(
            const objectrecognition::ClickLocation& click_location,
            const std::vector<mediapipe::Detection>& detections);

    static std::unique_ptr<mediapipe::ImageFrame> CropImage(
            const mediapipe::ImageFrame& image_frame,
            const mediapipe::Detection& detection);
};

REGISTER_CALCULATOR(BoundaryBoxCropCalculator);

mediapipe::Status BoundaryBoxCropCalculator::GetContract(mediapipe::CalculatorContract *cc) {
    cc->Inputs().Get("DETECTION", 0).Set<std::vector<mediapipe::Detection>>();
    cc->Inputs().Get("IMAGE", 0).Set<mediapipe::ImageFrame>();
    cc->Inputs().Get("CLICK", 0).Set<std::string>(); // objectdetection::ClickLocation

    cc->Outputs().Get("", 0).Set<mediapipe::ImageFrame>();

    return mediapipe::OkStatus();
}

mediapipe::Status BoundaryBoxCropCalculator::Open(mediapipe::CalculatorContext *cc) {
    cc->SetOffset(::mediapipe::TimestampDiff(0));
    return mediapipe::OkStatus();
}

mediapipe::Status BoundaryBoxCropCalculator::Process(mediapipe::CalculatorContext *cc) {
    auto& detections_packet = cc->Inputs().Get("DETECTION", 0);
    auto& frames_packet = cc->Inputs().Get("IMAGE", 0);
    auto& click_packet = cc->Inputs().Get("CLICK", 0);

    if (detections_packet.IsEmpty()
        || frames_packet.IsEmpty()
        || click_packet.IsEmpty()) {
        return mediapipe::OkStatus();
    }

    const std::vector<mediapipe::Detection>& detections =
            detections_packet.Get<std::vector<mediapipe::Detection>>();
    const mediapipe::ImageFrame& image_frame = frames_packet.Get<mediapipe::ImageFrame>();

    // Java код записывает сериализованный protobuf как строку, поэтому его нужно вручную разбирать.
    auto click_location_str = click_packet.Get<std::string>();
    objectrecognition::ClickLocation click_location;
    click_location.ParseFromString(click_location_str);
    // Нажатия не было, выходим без дальнейшей обработки
    if (click_location.x() == -1 || click_location.y() == -1) {
        return mediapipe::OkStatus();
    }

    // Определение по какой рамке было нажатие
    absl::optional<mediapipe::Detection> detection = FindOverlappedDetection(click_location, detections);

    if (detection.has_value()) {
        // если нажатие было по рамке, то изображение внутри рамки вырезается
        std::unique_ptr<mediapipe::ImageFrame> cropped_image = CropImage(image_frame, detection.value());
        cc->Outputs().Get("", 0).Add(cropped_image.release(), cc->InputTimestamp());
    }

    return mediapipe::OkStatus();
}

absl::optional<mediapipe::Detection> BoundaryBoxCropCalculator::FindOverlappedDetection(
        const objectrecognition::ClickLocation& click_location,
        const std::vector<mediapipe::Detection>& detections) {
    for (const auto& input_detection : detections) {
        const auto& b_box = input_detection.location_data().relative_bounding_box();

        if (b_box.xmin() < click_location.x() && click_location.x() < (b_box.xmin() + b_box.width())
            && b_box.ymin() < click_location.y() && click_location.y() < (b_box.ymin() + b_box.height())) {
            return input_detection;
        }
    }

    return absl::nullopt;
}

std::unique_ptr<mediapipe::ImageFrame> BoundaryBoxCropCalculator::CropImage(
        const mediapipe::ImageFrame& image_frame,
        const mediapipe::Detection& detection) {
    const uint8* pixel_data = image_frame.PixelData();
    const auto& b_box = detection.location_data().relative_bounding_box();

    int height = static_cast<int>(b_box.height() * static_cast<float>(image_frame.Height()));
    int width = static_cast<int>(b_box.width() * static_cast<float>(image_frame.Width()));
    int xmin = static_cast<int>(b_box.xmin() * static_cast<float>(image_frame.Width()));
    int ymin = static_cast<int>(b_box.ymin() * static_cast<float>(image_frame.Height()));

    if (xmin < 0) {
        width += xmin;
        xmin = 0;
    }
    if (ymin < 0) {
        height += ymin;
        ymin = 0;
    }
    if (width > image_frame.Width()) {
        width = image_frame.Width();
    }
    if (height > image_frame.Height()) {
        height = image_frame.Height();
    }

    std::vector<uint8_t> pixels;
    pixels.reserve(height * width * image_frame.NumberOfChannels());
    for (int y = ymin; y < ymin + height; ++y) {
        int row_offset = y * image_frame.WidthStep();
        for (int x = xmin; x < xmin + width; ++x) {
            for (int ch = 0; ch < image_frame.NumberOfChannels(); ++ch) {
                pixels.push_back(pixel_data[row_offset + x * image_frame.NumberOfChannels() + ch]);
            }
        }
    }

    std::unique_ptr<mediapipe::ImageFrame> cropped_image = std::make_unique<mediapipe::ImageFrame>();
    cropped_image->CopyPixelData(image_frame.Format(), width, height, pixels.data(),
            mediapipe::ImageFrame::kDefaultAlignmentBoundary);

    return cropped_image;
}
