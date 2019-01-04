
import os
import sys
import binascii
import logging
import logging.handlers
from urllib.parse import parse_qs
from wsgiref.simple_server import make_server, WSGIServer, WSGIRequestHandler
from bsides2019.token import Token
from bsides2019.scoredb import ScoreDb
from bsides2019.html import HtmlTable

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


def debug_mode():
    try:
        if not 'NOPIA_DEBUG' in os.environ:
            return False
        if os.environ['NOPIA_DEBUG'].lower() != 'true':
            return False
    except:
        return False
    return True

def db_example(environ, start_response):
    data = 'db'
    with ScoreDb(version=0) as score:
        #score.add_score('foobar1', 'test', 100)
        #score.add_score('foobar2', 'test',200)
        #score.add_score('foobar3', 'test', 300)
        data += ' ' + score.sql_version()
        data += ' ' + HtmlTable(score.score_board('test')).html()
        #score.add_name('foobar1', 'lalala')
        data += ' ' + HtmlTable(score.name_table()).html()
    start_response('200 OK', [('Content-Type','text/html'), ('Content-Length',str(len(data)))])
    return [data]

def err404_app(environ, start_response, err='nopia'):
    data = err.encode('ascii')
    start_response('404 Not Found', [('Content-Type','text/html'), ('Content-Length',str(len(data)))])
    return [data]

def bsides2019_app(environ, start_response):
    key = binascii.a2b_hex(os.environ['NOPIA_KEY'])
    assert len(key) == 16
    token = Token.decode(key, parse_qs(environ['QUERY_STRING'])['t'][0])
    text = '<html><head></head><body><h1>{}</h1><p>{} {} pts</p></body></html>'.format(token.imei, token.game_name, token.score)
    data = text.encode('utf-8')
    start_response('200 OK', [('Content-Type','text/html'), ('Content-Length',str(len(data)))])
    return [data]

def application(environ, start_response):
    err = ''
    try:
        if environ['PATH_INFO'].endswith('db.html'):
            return db_example(environ, start_response)
        return bsides2019_app(environ, start_response)
    except Exception as error:
        import traceback
        if debug_mode():
            err = str(traceback.format_exc())
        else:
            err = 'nopia'
    return err404_app(environ, start_response, err)

if __name__ == '__main__':
    server = WSGIServer
    handler = NoReversLookupsWSGIRequestHandler
    server_address = ("0.0.0.0", 3117)
    httpd = server(server_address, handler)
    httpd.set_app(application)
    print("Serving on port 3117...")
    httpd.serve_forever()
