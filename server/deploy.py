import os
import sys
import zipfile
import binascii

MASTER_KEY_HEX = os.path.join(os.path.dirname(__file__), '..', 'keys', 'master.hex')
assert os.path.exists(MASTER_KEY_HEX)
with open(MASTER_KEY_HEX, 'rt') as handle:
    MASTER_KEY = binascii.a2b_hex(handle.read())
assert isinstance(MASTER_KEY, bytes)
assert len(MASTER_KEY) == 16

APPLICATION_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), 'nopia'))
assert os.path.exists(APPLICATION_DIR)
assert os.path.isdir(APPLICATION_DIR)

ARTIFACTS_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'artifacts'))
if not os.path.exists(ARTIFACTS_DIR):
    os.mkdir(ARTIFACTS_DIR)
assert os.path.exists(ARTIFACTS_DIR)
assert os.path.isdir(ARTIFACTS_DIR)

APPLICATION_ZIP = os.path.join(ARTIFACTS_DIR, 'nopia-eb.zip')

EB_CONFIG = os.path.join(os.path.dirname(__file__), '..', '.elasticbeanstalk', 'config.yml')
with open(EB_CONFIG, 'rt') as handle:
    text = handle.read()
text = text.replace('\n', '').replace('\t','').replace(' ', '')
artifact_expected = os.path.relpath(APPLICATION_ZIP, os.path.join(os.path.dirname(__file__), '..')).replace('\\', '/')
assert 'deploy:artifact:' + artifact_expected in text, "Please add 'deploy: artifact: {}' to elasticbeanstalk config.yml".format(artifact_expected)

def eb(args):
    assert 0 == os.system('eb ' + ' '.join(args))

def eb_setenv(key, value):
    print('Setting {}={}'.format(key, value))
    eb(['setenv', '{}={}'.format(key, value)])

def eb_deploy():
    print('Deploying {}'.format(APPLICATION_ZIP))
    eb(['deploy', '--staged'])

def load_master_key():
    key_hex = binascii.b2a_hex(MASTER_KEY).decode('ascii')
    assert isinstance(key_hex, str) and not isinstance(key_hex, bytes)
    assert len(key_hex) == 32
    eb_setenv('NOPIA_KEY', key_hex)

def loadkeys():
    load_master_key()

def package():
    if os.path.exists(APPLICATION_ZIP):
        print('Removing {}'.format(APPLICATION_ZIP))
        os.remove(APPLICATION_ZIP)
    print('Creating {}'.format(APPLICATION_ZIP))
    with zipfile.ZipFile(APPLICATION_ZIP, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(APPLICATION_DIR):
            for file in files:
                if file.startswith('.'):
                    continue
                path = os.path.join(root, file)
                if path.find('__pycache__') != -1:
                    continue
                arcname = os.path.relpath(path, APPLICATION_DIR).replace('\\', '/')
                print('Adding {}'.format(arcname))
                zipf.write(path, arcname=arcname)
        print('Saving {}'.format(APPLICATION_ZIP))
    assert os.path.exists(APPLICATION_ZIP)

def deploy():
    package()
    eb_deploy()

ACTIONS = {
    'loadkeys': loadkeys,
    'package': package,
    'deploy': deploy,
}

def usage():
    print(os.path.basename(__file__) + ' <' + '|'.join(sorted(ACTIONS.keys())) + '>')

ACTIONS['usage'] = usage

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        action = usage
    else:
        action = ACTIONS.get(sys.argv[1], 'usage')
    action()
    sys.exit(0)
