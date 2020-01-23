#include <android/log.h>
#include "mediapipe//framework/packet.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_format.pb.h"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/Base64Encoder.h>
#include <Poco/JSON/Parser.h>
#include <thread>

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(const std::vector<uint8>& bytes) {
    std::string ret;
    int i = 0;
    int j = 0;
    uint8 char_array_3[3];
    uint8 char_array_4[4];

    for (auto byte : bytes) {
        char_array_3[i++] = byte;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;

}
std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

struct ImageData {
    std::vector<uint8> pixels;
    int width;
    int height;
};

Poco::JSON::Object ConvertToJSONObject(const ImageData& image_data) {
    Poco::JSON::Object object;

    object.set("pixels", base64_encode(image_data.pixels));
    object.set("width", image_data.width);
    object.set("height", image_data.height);

    return object;
}


class DetectionToTextCalculator : public ::mediapipe::CalculatorBase {
public:
    static ::mediapipe::Status GetContract(::mediapipe::CalculatorContract* cc);

    ::mediapipe::Status Open(::mediapipe::CalculatorContext* cc) override;

    ::mediapipe::Status Process(::mediapipe::CalculatorContext* cc) override;

private:
    std::unordered_map<int, std::string> label_map_;

    static void SendImagesToServer(const std::vector<ImageData>& images);
};

REGISTER_CALCULATOR(DetectionToTextCalculator);

::mediapipe::Status DetectionToTextCalculator::GetContract(::mediapipe::CalculatorContract *cc) {
    cc->Inputs().Get("", 0).Set<std::vector<::mediapipe::Detection>>();
    cc->Outputs().Get("", 0).Set<std::vector<::mediapipe::Detection>>();

    cc->Inputs().Tag("INPUT_FRAME_GPU").Set<mediapipe::ImageFrame>();

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

//    auto& string_packet = cc->Inputs().Get("SECOND", 0);
//    if (!string_packet.IsEmpty()) {
//        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", string_packet.Get<std::string>().c_str());
//    }

    auto& frames_packet = cc->Inputs().Get("INPUT_FRAME_GPU", 0);

    if (!frames_packet.IsEmpty()) {
        const mediapipe::ImageFrame& image_frame = frames_packet.Get<mediapipe::ImageFrame>();
        // std::unique_ptr<uint8> buffer = std::unique_ptr<uint8>(new uint8[image_frame.PixelDataSize()]);
        // image_frame.CopyToBuffer(buffer.get(), image_frame.PixelDataSize());

//        __android_log_print(
//                ANDROID_LOG_ERROR,
//                "TRACKERS",
//                "WxHxD %dx%dx%d, BD=%d, PDS=%d, PDSSC=%d, WS=%d",
//                image_frame.Width(),
//                image_frame.Height(),
//                image_frame.NumberOfChannels(),
//                image_frame.ByteDepth(),
//                image_frame.PixelDataSize(),
//                image_frame.PixelDataSizeStoredContiguously(),
//                image_frame.WidthStep());

        const uint8* pixel_data = image_frame.PixelData();
        auto detections = cc->Inputs().Get("", 0).Get<std::vector<::mediapipe::Detection>>();
        std::vector<ImageData> cropped_images;
        cropped_images.reserve(detections.size());

        for (const auto& input_detection : detections) {
            const auto& b_box = input_detection.location_data().relative_bounding_box();

            int height = b_box.height() * image_frame.Height();
            int width = b_box.width() * image_frame.Width();
            int xmin = b_box.xmin() * image_frame.Width();
            int ymin = b_box.ymin() * image_frame.Height();

            __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "bbox2 WxH %dx%d, xmin ymin %dx%d",
                            width, height, xmin, ymin);

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

            cropped_images.emplace_back();
            auto& cropped_image = cropped_images.back();
            cropped_image.pixels.reserve(height * width * image_frame.NumberOfChannels());
            cropped_image.width = width;
            cropped_image.height = height;

            for (int y = ymin; y < ymin + height; ++y) {
                int row_offset = y * image_frame.WidthStep();
                for (int x = xmin; x < xmin + width; ++x) {
                    for (int ch = 0; ch < image_frame.NumberOfChannels(); ++ch) {
                        cropped_image.pixels.push_back(pixel_data[row_offset + x * image_frame.NumberOfChannels() + ch]);
                    }
                }
            }


            __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "ci.size() == %d", cropped_image.pixels.size());
        }

//        cropped_images.emplace_back();
//        auto& cropped_image = cropped_images.back();
//        cropped_image.pixels.reserve(100 * 100 * image_frame.NumberOfChannels());
//        cropped_image.width = 100;
//        cropped_image.height = 100;
//
//        for (int y = 0; y < 100; ++y) {
//            int row_offset = y * image_frame.WidthStep();
//            for (int x = 0; x < 100; ++x) {
//                for (int ch = 0; ch < image_frame.NumberOfChannels(); ++ch) {
//                    cropped_image.pixels.push_back(pixel_data[row_offset + x * image_frame.NumberOfChannels() + ch]);
//                }
//            }
//        }

//        for (int i = 0; i < image_frame.PixelDataSizeStoredContiguously(); ++i) {
//            cropped_image.pixels.push_back(image_frame.PixelData()[i]);
//        }

        if (!cropped_images.empty()) {
            SendImagesToServer(cropped_images);
        }
    }

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

void DetectionToTextCalculator::SendImagesToServer(const std::vector<ImageData>& images) {
    Poco::JSON::Array json_array;
    int index = 0;
    for (const auto& image : images) {
        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%d %d %d", image.pixels.size(), image.width, image.height);
        json_array.set(index, ConvertToJSONObject(image));
        ++index;
    }

    Poco::JSON::Object object;
    object.set("images", json_array);

    std::ostringstream oss;
    object.stringify(oss);
    std::string payload = oss.str();
    // __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "payload = %s", payload.c_str());
    //payload = "{}";
    try {
        std::string url = "http://192.168.1.122:8080/ppp";
        Poco::URI uri(url);
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());

        Poco::Net::HTTPRequest request("POST", uri.getPathAndQuery(), "HTTP/1.1");
        request.add("Content-Length", std::to_string(payload.size()));
        request.add("Content-Type", "application/json");
        auto& request_stream = session.sendRequest(request);
        request_stream << payload;

        Poco::Net::HTTPResponse response;
        auto& body = session.receiveResponse(response);
        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "sent %d", response.getStatus());
    } catch (...){
        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "Network error");
    }


}