#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include <fstream>
#include <iostream>

#define TFLITE_MINIMAL_CHECK(x)                              \
  if (!(x)) {                                                \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
    exit(1);                                                 \
  }

const int kImageSize = 320 * 320 * 3;

std::vector<float> ReadMatrix(const std::string& filename) {
    std::ifstream input_file(filename);
    std::vector<float> vector;

    vector.reserve(kImageSize);

    for (int i = 0; i < kImageSize; ++i) {
        vector.emplace_back();
        input_file >> vector.back();
    }

    input_file.close();

    return vector;
}

void WriteToFile(const std::string& filename, float* array, int size) {
    std::ofstream output_file(filename);

    for (int i = 0; i < size; ++i) {
        output_file << array[i] << " ";
    }

    output_file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <tflite file> <image matrix file> <output prefix file>\n";
        return 1;
    }

    std::string tflite_file = std::string(argv[1]);
    std::string matrix_file = std::string(argv[2]);
    std::string output_prefix = std::string(argv[3]);

    auto matrix = ReadMatrix(matrix_file);

    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(tflite_file.c_str());
    TFLITE_MINIMAL_CHECK(model != nullptr);

    // Build the interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    builder(&interpreter);
    TFLITE_MINIMAL_CHECK(interpreter != nullptr);

    // Allocate tensor buffers.
    TFLITE_MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);

    // Fill input buffers
    float* input_tensor = interpreter->typed_input_tensor<float>(0);
    memcpy(input_tensor, matrix.data(), kImageSize * sizeof(float));

    // Run inference
    TFLITE_MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);

    // Read output buffers
    float* tensor = interpreter->typed_tensor<float>(interpreter->outputs()[0]);
    WriteToFile(output_prefix + "-box_encodings.txt", tensor, 2034 * 4);

    float* tensor2 = interpreter->typed_output_tensor<float>(1);
    WriteToFile(output_prefix + "-class_predictions.txt", tensor2, 2034 * 91);

    return 0;
}