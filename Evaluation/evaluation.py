import tflite_runtime.interpreter as tflite
import sys
import numpy as np

if len(sys.argv) != 4:
    print(f'Usage: {sys.argv[0]} <tflite file> <image matrix file> <output prefix file>')
    exit(1)

tflite_file = sys.argv[1]
image_matrix_file = sys.argv[2]
output_prefix = sys.argv[3]

arr = np.loadtxt(image_matrix_file, dtype=np.float32)
arr = arr.reshape(1, 320, 320, 3)

interpreter = tflite.Interpreter(model_path=tflite_file)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

input_shape = input_details[0]['shape']
interpreter.set_tensor(input_details[0]['index'], arr)

interpreter.invoke()

output_data0 = interpreter.get_tensor(output_details[0]['index'])
output_data1 = interpreter.get_tensor(output_details[1]['index'])

np.savetxt(f'{output_prefix}-box_encodings.txt', output_data0.reshape(-1))
np.savetxt(f'{output_prefix}-class_predictions.txt', output_data1.reshape(-1))