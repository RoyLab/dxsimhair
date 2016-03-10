import nCache
import numpy as np


def importFile(fileName):
    hk = Hooker()
    nCache.importFile(fileName, hk)
    return hk.get_data()

class Hooker:

    def __init__(self):
        self.frames = []
        self.cframe = None

    def data_hooker(self, name, sz, arr):
        # print sz, arr[0:5], len(arr), name
        self.cframe.loadIntoMemory(name, sz, arr)

    def new_frame(self):
        self.cframe = Frame()
        self.frames.append(self.cframe)

    def get_data(self):
        return self.frames


class Frame:

    def __init__(self):
        self.count = 0
        self.data = None
        self.headData = None

        self.n_headVertex = 0
        self.n_hair = 0
        self.n_particle = 0

    def loadIntoMemory(self, name, sz, data):
        if self.count == 0:
            self.n_headVertex = int(sz)
            self.headData = data
        elif self.count == 1:
            # ipdb.set_trace()
            self.n_hair = int(data[0])
            self.n_particle = self.n_hair * 25
        elif self.count == 3:
            self.data = data

        self.count += 1
