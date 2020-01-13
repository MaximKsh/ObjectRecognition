#include "mediapipe/framework/calculator_framework.h"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>

class HttpTestCalculator : public mediapipe::CalculatorBase {
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
        auto url = cc->Inputs().Index(0).Value().Get<std::string>();

        Poco::URI uri(url);
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());

        Poco::Net::HTTPRequest request("GET", uri.getPathAndQuery(), "HTTP/1.1");
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        auto& body = session.receiveResponse(response);
        std::string text_body(std::istreambuf_iterator<char>(body), {});

        auto packet = mediapipe::MakePacket<std::string>(text_body).At(cc->InputTimestamp());
        cc->Outputs().Index(0).AddPacket(packet);

        return mediapipe::OkStatus();
    }

private:

};

REGISTER_CALCULATOR(HttpTestCalculator);
