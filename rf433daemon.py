#!/usr/bin/python3

import configparser
import serial
import io
import logging
import subprocess
import shlex

config = configparser.ConfigParser()
config.read('eray.conf')
config.sections()

def dump_config():
    for key in config['devices']:
        print(key + " " + config['devices'][key])

    for key in config['commands']:
        print(key + " " + config['commands'][key])

    for key in config['actions']:
        print(key + " " + config['commands'][key])

ser = serial.Serial('/dev/ttyUSB0', 9600)

logger = logging.getLogger("ERAY")
logger.setLevel(logging.INFO)

ch = logging.StreamHandler()
formatter = logging.Formatter("%(asctime)s %(message)s")
ch.setFormatter(formatter)
logger.addHandler(ch)

while True:
    line = ser.readline()
    try:
        value = int(line, 16)
    except:
        logger.info('Unknown event: ' + str(line) )
        continue

    device = value >> 8
    command = value & 0xff

    try:
        device_name = config['devices'][format(device, '04X')]
    except:
        device_name = "(Unknown device)"

    try:
        command_name = config['commands'][format(command, '02X')]
    except:
        command_name = "(Unknown command)"

    id = format(value, '06X')

    logger.info(id + ' ' + device_name + ': ' + command_name)

    try:
        action = config['actions'][id]
        try:
            subprocess.run(shlex.split(action))
            logger.info('Executed: ' + action)
        except:
            logger.info('Cannot execute: ' + action)
    except Exception:
        pass
