from aiohttp import web
from PIL import Image
import base64
import datetime

async def handle(request):
    req_body = await request.json()

    for image in req_body['images']:
        width, height = image['width'], image['height']
        bts = bytearray(image['pixels'], 'UTF8')
        rgba = base64.decodestring(bts)

        print(len(rgba), width, height)
        img = Image.frombuffer('RGBA', (width, height, ), rgba, 'raw', 'RGBA', 0, 1)
        img.save(f'/home/maxim/Pictures/111/img-{datetime.datetime.now()}.png')

    return web.Response(status=200)

async def gethandle(request):
    print('test')
    return web.Response(status=200)

app = web.Application(client_max_size=0)
app.add_routes([web.post('/ppp', handle), web.get('/ggg', gethandle)])

if __name__ == '__main__':
    web.run_app(app)
