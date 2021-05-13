# -*- coding: utf-8 -*-
"""
JUST FOR TEST.
"""
import os
import sys
import json
import uuid
import base64
import shutil
import signal
import requests
import subprocess

def run_cmd(cmd, cwd):
    res = {}
    str_res = ''

    try:
        res = subprocess.run(
            cmd,
            cwd=cwd,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            encoding="utf-8",
            timeout=600
        )
        print(res)
        print(res.returncode)
        print(res)

        if 0 == res.returncode:
            str_res = str(res.stdout)
        else:
            str_res = str(res.stderr)

    except KeyboardInterrupt:
        print('receive ctrl+c or SIGINT signal')

    except:
        print('exit') 

    return str_res

if __name__ == '__main__':
    currentDir, _ = os.path.split(os.path.abspath(__file__))
    run_cmd("sh timeout.sh", currentDir)
