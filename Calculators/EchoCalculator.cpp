#include "mediapipe/framework/calculator_framework.h"

class EchoCalculator : public mediapipe::CalculatorBase {
public:
    static mediapipe::Status GetContract(mediapipe::CalculatorContract* cc) {
        for (mediapipe::CollectionItemId id = cc->Inputs().BeginId();
             id < cc->Inputs().EndId(); ++id) {
            cc->Inputs().Get(id).SetAny();
            cc->Outputs().Get(id).SetSameAs(&cc->Inputs().Get(id));
        }

        return mediapipe::OkStatus();
    }

    mediapipe::Status Open(mediapipe::CalculatorContext* cc) final {

        return mediapipe::OkStatus();
    }

    mediapipe::Status Process(mediapipe::CalculatorContext* cc) final {
        auto txt = cc->Inputs().Index(0).Value().Get<std::string>();

        auto packet = mediapipe::MakePacket<std::string>(txt).At(cc->InputTimestamp());
        cc->Outputs().Index(0).AddPacket(packet);

        return mediapipe::OkStatus();
    }

private:

};

REGISTER_CALCULATOR(EchoCalculator);
