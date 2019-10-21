from ctypes import *
import os

__LIB__ = None

class OctoSQL:
    def __init__(self):
        global __LIB__
        if __LIB__ is None:
            libpath = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + "/../../build/libadd.so")
            __LIB__ = cdll.LoadLibrary(libpath)
        self.lib = __LIB__

    def lolxd(self):
        self.lib.lolxd()