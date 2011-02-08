#!/usr/bin/python
# -*- coding: utf-8 -*-


import sys
sys.path.append('./lib')

import voxyweb
import voxyproxy


voxyweb.start()
voxyproxy.run()

