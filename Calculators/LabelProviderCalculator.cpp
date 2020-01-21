#include <android/log.h>
#include "mediapipe//framework/packet.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"


std::string label_map_string = R"(???
person
bicycle
car
motorcycle
airplane
bus
train
truck
boat
traffic light
fire hydrant
???
stop sign
parking meter
bench
bird
cat
dog
horse
sheep
cow
elephant
bear
zebra
giraffe
???
backpack
umbrella
???
???
handbag
tie
suitcase
frisbee
skis
snowboard
sports ball
kite
baseball bat
baseball glove
skateboard
surfboard
tennis racket
bottle
???
wine glass
cup
fork
knife
spoon
bowl
banana
apple
sandwich
orange
broccoli
carrot
hot dog
pizza
donut
cake
chair
couch
potted plant
bed
???
dining table
???
???
toilet
???
tv
laptop
mouse
remote
keyboard
cell phone
microwave
oven
toaster
sink
refrigerator
???
book
clock
vase
scissors
teddy bear
hair drier
toothbrush
)";

class LabelProviderCalculator : public ::mediapipe::CalculatorBase {
public:
    static ::mediapipe::Status GetContract(::mediapipe::CalculatorContract* cc) {
        cc->Inputs().Index(0).Set<std::string>();
        cc->Outputs().Index(0).Set<std::string>();

        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status Open(::mediapipe::CalculatorContext* cc) override {
        return ::mediapipe::OkStatus();
    }


    ::mediapipe::Status Process(::mediapipe::CalculatorContext* cc) override {
        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", "ggggg");
        if (!cc->Inputs().Get("", 0).Value().IsEmpty()) {
            auto value = cc->Inputs().Get("", 0).Get<std::string>();
            __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", value.c_str());
            cc->Outputs().Index(0).AddPacket(::mediapipe::MakePacket<std::string>(value).At(cc->InputTimestamp()));
        }

        return ::mediapipe::OkStatus();
    }

private:
};

REGISTER_CALCULATOR(LabelProviderCalculator);
