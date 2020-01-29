import numpy as np
import numpy.linalg as sla

goldenset_box_encoding = np.loadtxt('data/python-box_encodings.txt', dtype=np.float32)
android_box_encoding = np.loadtxt('data/android-box_encodings.txt', dtype=np.float32)
desktop_box_encoding = np.loadtxt('data/desktop-box_encodings.txt', dtype=np.float32)

print('Box encoding')
n = sla.norm(android_box_encoding - goldenset_box_encoding)
print(f'Goldenset-android norm {n}')
n = sla.norm(desktop_box_encoding - goldenset_box_encoding)
print(f'Goldenset-desktop norm {n}')

goldenset_class_prediction = np.loadtxt('data/python-class_predictions.txt', dtype=np.float32)
android_class_prediction = np.loadtxt('data/android-class_predictions.txt', dtype=np.float32)
desktop_class_prediction = np.loadtxt('data/desktop-class_predictions.txt', dtype=np.float32)

print('Class prediction')
n = sla.norm(android_class_prediction - goldenset_class_prediction)
print(f'Goldenset-android norm {n}')
n = sla.norm(desktop_class_prediction - goldenset_class_prediction)
print(f'Goldenset-desktop norm {n}')
