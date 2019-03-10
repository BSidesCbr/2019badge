import os
import sys
import time
import subprocess


def which(path):
    bin = 'which'
    if os.name == 'nt':
        bin = 'where'
    output = subprocess.check_output([bin, path])
    output = output.decode().strip()
    output = output.split('\n')[0]
    assert os.path.exists(output)
    return output

def avrdude_version(path):
    output = subprocess.check_output([path], stderr=subprocess.STDOUT)
    output = output.decode().strip()
    for line in output.split('\n'):
        line = line.strip()
        if line.find('version') == -1:
            continue
        line = line.split('version')[1]
        if line.find(',') == -1:
            continue
        line = line.split(',')[0]
        line = line.strip()
        if line.count('.') == 1:
            line += '.0'
        assert line.count('.') == 2
        major, minor, build = line.split('.')
        major = int(major)
        minor = int(minor)
        build = int(build)
        return (major, minor, build)
    raise AssertionError("avrdude version not found")

def avrdude_conf(path):
    path = path.replace('\\', '/')
    assert '/tools/avr/' in path
    path = path.split('/tools/avr/')[0] + '/tools/avr/etc/avrdude.conf'
    if os.name == 'nt':
        path = path.replace('/', '\\')
    assert os.path.exists(path)
    return path

def avrdude_flash(avrdude, conf, firmware):
    ret = 1
    while ret != 0:
        args = list()
        # usbasp
        # avrisp2
        args.extend([avrdude, '-C', conf, '-s', '-v', '-p', 'atmega328p', '-c', 'avrisp2', '-P', 'usb', '-U', 'flash:w:{}:i'.format(firmware)])
        args.extend(['-U', 'lfuse:w:0xD6:m', '-U', 'hfuse:w:0xD9:m', '-U', 'efuse:w:0xFF:m'])
        ret = subprocess.call(args)
        time.sleep(1)
    print('******* SUCCESS *********')

avrdude = 'avrdude'
avrdude = which(avrdude)
version = avrdude_version(avrdude)
conf = avrdude_conf(avrdude)
firmware = os.path.realpath(os.path.join(os.path.dirname(__file__), 'bsides2019', 'bsides2019.ino.standard.hex'))
input("Press Enter to flash...")
while True:
    avrdude_flash(avrdude, conf, firmware)
    #input("Press Enter to flash next...")
    time.sleep(5)

