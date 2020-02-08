from aiohttp import web
from PIL import Image
import base64
import datetime

async def test(request):
    return web.Response(text='Backend service is available', status=200)

async def handle(request):
    image = await request.json()

    width, height = image['width'], image['height']
    bts = bytearray(image['base64_pixels'], 'UTF8')
    rgba = base64.decodestring(bts)

    print(len(rgba), width, height)
    img = Image.frombuffer('RGBA', (width, height, ), rgba, 'raw', 'RGBA', 0, 1)
    img.save(f'/home/maxim/Pictures/111/img-{datetime.datetime.now()}.png')

    response = {
        'label': 'unknown car',
    }

    return web.json_response(response)

async def gethandle(request):
    print('test')
    return web.Response(status=200)

app = web.Application(client_max_size=0)
app.add_routes([web.post('/recognize', handle), web.get('/test', test)])

if __name__ == '__main__':
    web.run_app(app)
