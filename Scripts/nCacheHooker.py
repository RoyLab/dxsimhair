import nCache

def importFile(fileName, number=5):
    hk = Hooker(number)
    nCache.importFile(fileName, hk, number)
    hk.bar.finish()
    return hk.get_data(), hk.hash

class Hooker:

    def __init__(self, number):
        self.frames = []
        self.cframe = None
        self.hash = {}
        self.n = number
        self.bar =  ProgressBar().start()
        self.count = 0
        self.count4 = 0

    def data_hooker(self, name, sz, arr):
        self.cframe.loadIntoMemory(name, sz, arr)
        self.count4 += 1

        if self.count4 % 4 == 0:
            self.count4 = 0
            createInitGraph_loop(self.cframe, self.hash, self.count)
            self.count += 1
            self.bar.update(self.count*100/self.n)

    def new_frame(self):
        self.cframe = Frame()
        self.frames.append(self.cframe)

    def get_data(self):
        return self.frames
