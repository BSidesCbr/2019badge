
import os
import sys
import string
import binascii
from urllib.parse import parse_qs
from bsides2019.token import Token
from bsides2019.scoredb import ScoreDb
from bsides2019.html import HtmlTable

def debug_mode():
    try:
        if not 'NOPIA_DEBUG' in os.environ:
            return False
        if os.environ['NOPIA_DEBUG'].lower() != 'true':
            return False
    except:
        return False
    return True

def get_version():
    return int(os.environ['NOPIA_DB_VERSION'])

def get_key():
    try:
        key = binascii.a2b_hex(os.environ['NOPIA_KEY'])
        assert len(key) == 16
        assert isinstance(key, bytes)
    except:
        return None
    return key

def get_query_string(environ, param):
    try:
        value = parse_qs(environ['QUERY_STRING'])[param][0]
    except:
        return None
    return value

def get_score_row_count(environ):
    return int(os.environ['NOPIA_SCORE_ROWS'])

def format_score_board(table):
    new_table = list()
    i = 0
    for row in table:
        imei = row[0]
        name = row[1]
        pos = '#{}'.format(i + 1)
        name = imei if not name else '{} [{}]'.format(imei, name)
        score = str(row[2]) + ' pts'
        new_row = [pos, name, score]
        new_table.append(new_row)
        i += 1
    table = new_table
    return table

def sanitize_name(name_unsafe):
    # these are the characters we will allow for names
    ok_chars = string.ascii_letters + string.digits + '_- '
    # no name, no name
    if not name_unsafe:
        return None
    # deal with ascii characters only (and hence make it bytes first)
    name = name_unsafe.encode('ascii')
    # no longer than 8 characters
    name = name[:8]
    # remove all the bad ones
    name_safe = ''
    for value in name:
        if chr(value) in ok_chars:
            name_safe += chr(value)
    # don't return an empty name
    if len(name_safe) == 0:
        return None
    return name_safe

def error_app(environ, start_response, err='nopia'):
    data = err.encode('ascii')
    start_response('200 OK', [('Content-Type','text/html'), ('Content-Length',str(len(data)))])
    return [data]

def success_app(environ, start_response, text):
    text = '<html><head></head><body>{}</body></html>'.format(text)
    data = text.encode('ascii')
    start_response('200 OK', [('Content-Type','text/html'), ('Content-Length',str(len(data)))])
    return [data]

def score_board_app(environ, start_response):
    text = ''
    with ScoreDb(version=get_version()) as score:
        text += '<h1>{}</h1>'.format('Snake')
        text += HtmlTable(format_score_board(score.score_board('snake', count=get_score_row_count(environ)))).html()
        text += '<h1>{}</h1>'.format('Tetris')
        text += HtmlTable(format_score_board(score.score_board('tetris', count=get_score_row_count(environ)))).html()
    return success_app(environ, start_response, text)

def submit_name_form_app(environ, start_response, qs_token, token):
    text = ''
    text += '<h1>{}</h1><p>{} {} pts</p>'.format(token.imei, token.game, token.score)
    text += '<form id="name_form" "/index.html" method="get">'
    text += '<input type="hidden" name="s" value="{}" />'.format(qs_token)
    text += '<input type="text" name="n" value="" />'
    text += '<button type="submit" form="name_form" value="Submit">Submit</button>'
    text += '</form>'
    return success_app(environ, start_response, text)

def bsides2019_app(environ, start_response):
    key = get_key()
    qs_token_unsafe = get_query_string(environ, 't')
    qs_save_unsafe = get_query_string(environ, 's')
    qs_name_unsafe = get_query_string(environ, 'n')
    if environ['PATH_INFO'].endswith('reset.html'):
        # only allow a reset in debug mode
        if debug_mode():
            with ScoreDb(version=get_version()) as score:
                score.reset_db()
    if qs_token_unsafe:
        # uploading a token
        token = Token.decode(key, qs_token_unsafe)
        # if no error parsing the token, its considered safe
        qs_token_safe = qs_token_unsafe
        # add the score
        with ScoreDb(version=get_version()) as score:
            score.add_score(token.imei, token.game, token.score)
        # return a form for the user to set their name
        return submit_name_form_app(environ, start_response, qs_token_safe, token)
    elif qs_save_unsafe:
        # parse the token to get the user imei
        token = Token.decode(key, qs_save_unsafe)
        # second time (with a name field) change the name
        safe_name = sanitize_name(qs_name_unsafe)
        if safe_name:
            with ScoreDb(version=get_version()) as score:
                score.add_name(token.imei, safe_name)
        # fall through to score board
    return score_board_app(environ, start_response)

def application(environ, start_response):
    err = ''
    try:
        return bsides2019_app(environ, start_response)
    except Exception as error:
        # application failed
        import traceback
        if debug_mode():
            # show the stack trace in debug mode
            err = str(traceback.format_exc())
        else:
            # show the score board otherwise
            try:
                return score_board_app(environ, start_response)
            except:
                # the score board failed, display nothing
                err = ''
    return error_app(environ, start_response, err)
