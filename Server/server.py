from aiohttp import web
from PIL import Image
import base64
import datetime
import scipy.misc
import tensorflow as tf
import numpy as np
from tensorflow.keras.models import load_model

async def test(request):
    return web.Response(text='Backend service is available', status=200)

async def handle(request):
    image = await request.json()

    width, height = image['width'], image['height']
    bts = bytearray(image['base64_pixels'], 'UTF8')
    rgba = base64.decodestring(bts)

    img = Image.frombuffer('RGBA', (width, height, ), rgba, 'raw', 'RGBA', 0, 1)
    img = img.resize((192, 192))
    img = img.convert('RGB')
    x = tf.keras.preprocessing.image.img_to_array(img)
    x = np.expand_dims(x, axis=0)
    x = x / 255.0
 
    pred = np.array(model.predict(x)).reshape(-1)
    label = label_names[np.argmax(pred)]

    img.save(f'/home/maxim/Pictures/img-{datetime.datetime.now()}-{label}.png')

    response = {
        'label': label,
    }

    return web.json_response(response)


gpus = tf.config.experimental.list_physical_devices('GPU')
for gpu in gpus:
    tf.config.experimental.set_memory_growth(gpu, True)

label_names = np.load('label_names.npy')
model = load_model('cars.h5')

app = web.Application(client_max_size=0)
app.add_routes([web.post('/recognize', handle), web.get('/test', test)])

if __name__ == '__main__':
    web.run_app(app)
