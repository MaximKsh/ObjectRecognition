#include <android/log.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include "mediapipe//framework/packet.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "Utils/Base64.h"

struct ClickLocation {
    float x;
    float y;

    static ClickLocation FromJson(const std::string& json) {
        Poco::JSON::Parser parser;
        auto json_object = parser.parse(json);
        auto ptr = json_object.extract<Poco::JSON::Object::Ptr>();

        ClickLocation click_location;
        click_location.x = ptr->getValue<float>("x");
        click_location.y = ptr->getValue<float>("y");
        return click_location;
    }

    std::string ToJson() {
        Poco::JSON::Object json_object;
        json_object.set("x", x);
        json_object.set("y", y);
        std::ostringstream oss;
        json_object.stringify(oss);
        return oss.str();
    }
};

struct CroppedImage {
    std::string base64_pixels;
    int width;
    int height;

    static CroppedImage FromJson(const std::string& json) {
        Poco::JSON::Parser parser;
        auto json_object = parser.parse(json);
        auto ptr = json_object.extract<Poco::JSON::Object::Ptr>();

        CroppedImage cropped_image;
        cropped_image.base64_pixels = ptr->getValue<std::string>("base64_pixels");
        cropped_image.width = ptr->getValue<float>("width");
        cropped_image.height = ptr->getValue<float>("height");
        return cropped_image;
    }

    std::string ToJson() {
        Poco::JSON::Object json_object;
        json_object.set("base64_pixels", base64_pixels);
        json_object.set("width", width);
        json_object.set("height", height);
        std::ostringstream oss;
        json_object.stringify(oss);
        return oss.str();
    }
};

class BoundaryBoxCropCalculator : public mediapipe::CalculatorBase {
public:
    static mediapipe::Status GetContract(mediapipe::CalculatorContract* cc);

    mediapipe::Status Open(mediapipe::CalculatorContext* cc) override;

    mediapipe::Status Process(mediapipe::CalculatorContext* cc) override;

private:
    static absl::optional<mediapipe::Detection> FindOverlappedDetection(
            const ClickLocation& click_location,
            const std::vector<mediapipe::Detection>& detections);

    static CroppedImage CropImage(
            const mediapipe::ImageFrame& image_frame,
            const mediapipe::Detection& detection);
};

REGISTER_CALCULATOR(BoundaryBoxCropCalculator);

mediapipe::Status BoundaryBoxCropCalculator::GetContract(mediapipe::CalculatorContract *cc) {
    cc->Inputs().Get("DETECTION", 0).Set<std::vector<mediapipe::Detection>>();
    cc->Inputs().Get("IMAGE", 0).Set<mediapipe::ImageFrame>();
    cc->Inputs().Get("CLICK", 0).Set<std::string>(); // ClickLocation model

    cc->Outputs().Get("", 0).Set<std::string>(); // CroppedImage model

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

    const std::string& click_json = click_packet.Get<std::string>();
    //__android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", click_json.c_str());
    //__android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "Detections count %d", detections.size());
    if (click_json == "{}") {
        return mediapipe::OkStatus();
    }

    ClickLocation click_location = ClickLocation::FromJson(click_json);

    absl::optional<mediapipe::Detection> detection = FindOverlappedDetection(click_location, detections);

    if (detection.has_value()) {
        CroppedImage cropped_image = CropImage(image_frame, detection.value());

        std::string cropped_image_json = cropped_image.ToJson();
    // __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "CroppedImageJson %s", cropped_image_json.c_str());

        auto out_packet = mediapipe::MakePacket<std::string>(cropped_image_json).At(cc->InputTimestamp());
        cc->Outputs().Get("", 0).AddPacket(out_packet);
    }

    return mediapipe::OkStatus();
}

absl::optional<mediapipe::Detection> BoundaryBoxCropCalculator::FindOverlappedDetection(
        const ClickLocation& click_location,
        const std::vector<mediapipe::Detection>& detections) {
    for (const auto& input_detection : detections) {
        const auto& b_box = input_detection.location_data().relative_bounding_box();

        if (b_box.xmin() < click_location.x && click_location.x < (b_box.xmin() + b_box.width())
            && b_box.ymin() < click_location.y && click_location.y < (b_box.ymin() + b_box.height())) {
            return input_detection;
        }
    }

    return absl::nullopt;
}

CroppedImage BoundaryBoxCropCalculator::CropImage(
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

    CroppedImage cropped_image;
    cropped_image.width = width;
    cropped_image.height = height;
    cropped_image.base64_pixels = Base64::Encode(pixels);

    return cropped_image;
}
