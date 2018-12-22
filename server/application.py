
import os
import sys
import logging
import logging.handlers
from urllib.parse import parse_qs
from wsgiref.simple_server import make_server, WSGIServer, WSGIRequestHandler
from bsides2019.token import Token
from key import KEY

# Create logger
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# Handler
if os.name != 'nt':
    LOG_FILE = '/opt/python/log/sample-app.log'
    handler = logging.handlers.RotatingFileHandler(LOG_FILE, maxBytes=1048576, backupCount=5)
    handler.setLevel(logging.INFO)

    # Formatter
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    # Add Formatter to Handler
    handler.setFormatter(formatter)

    # add Handler to Logger
    logger.addHandler(handler)


class NoReversLookupsWSGIRequestHandler(WSGIRequestHandler):

    # Disable logging DNS lookups
    def address_string(self):
        return str(self.client_address[0])

def err404_app(environ, start_response):
    start_response('404 Not Found', [('Content-Type','text/html'), ('Content-Length',str(len(b'')))])
    return [b'']

def bsides2019_app(environ, start_response):
    token = Token.decode(KEY, parse_qs(environ['QUERY_STRING'])['t'][0])
    text = '<html><head></head><body><h1>{}</h1><p>{} {} pts</p></body></html>'.format(token.imei, token.game_name, token.score)
    data = text.encode('utf-8')
    start_response('200 OK', [('Content-Type','text/html'), ('Content-Length',str(len(data)))])
    return [data]

def application(environ, start_response):
    try:
        return bsides2019_app(environ, start_response)
    except Exception as error:
        import traceback
        print(traceback.format_exc())
        return err404_app(environ, start_response)

if __name__ == '__main__':
    server = WSGIServer
    handler = NoReversLookupsWSGIRequestHandler
    server_address = ("0.0.0.0", 3117)
    httpd = server(server_address, handler)
    httpd.set_app(bsides2019_app)
    print("Serving on port 3117...")
    httpd.serve_forever()
