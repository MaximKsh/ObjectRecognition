from aiohttp import web
from scipy import spatial
import numpy as np

async def test(request):
    return web.Response(text='Backend service is available', status=200)

async def handle(request):
    car_descriptor = await request.json()
    car = np.array(car_descriptor['car'])
    
    label = labels[np.argmin(np.array([spatial.distance.cosine(v, car) for v in vectors]))]

    response = {
        'label': label,
    }

    return web.json_response(response)

app = web.Application(client_max_size=0)
app.add_routes([web.post('/recognize', handle), web.get('/test', test)])

vectors = np.load('vectors.npy')
labels = np.load('labels.npy')

if __name__ == '__main__':
    web.run_app(app, host="127.0.0.1", port="10000")
