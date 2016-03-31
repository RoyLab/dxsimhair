from frame import Frame
from progressbar import *
from GraphBuilder import *

# def importFile(fileName, number=5):
#     hk = Hooker(number)
#     nCache.importFile(fileName, hk, number)
#     hk.bar.finish()
#     return hk.get_data(), hk.hash

class Hooker(object):
    def __init__(self, number=None):
        self.nFrame = number
        self.i = -1
        return

    def startLoop(self, title="start loop:"):
        print title
        self.bar =  ProgressBar().start()
        return

    def endLoop(self):
        self.bar.finish()

    def newFrame(self):
        self.frame = Frame()
        self.i += 1
        return

    def postFrame(self):
        self.bar.update((self.i+1)*100/self.nFrame)
        return

    def dataHooker(self, name, sz, arr):
        self.frame.loadIntoMemory(name, sz, arr)
        return

class GraphBuildHooker(Hooker):
    def __init__(self, radius):
        super(GraphBuildHooker, self).__init__()
        self.radius = radius
        self.edges = {}

    def postFrame(self):
        createInitGraphLoop(self.radius, self.frame, self.edges, self.i)
        if self.i == 0:
            self.refFrame = self.frame
            self.nParticle = self.frame.n_particle
            self.nStrand = self.frame.n_hair
        super(GraphBuildHooker, self).postFrame()

    def graph(self):
        return self.nStrand, self.nParticle, self.edges, self.refFrame

class ConnectionCalcHooker(Hooker):
    def __init__(self, edges, reference, prefix):
        super(ConnectionCalcHooker, self).__init__()
        self.edges = edges
        self.reference = reference
        self.prefix = prefix
        for k in edges.keys():
            edges[k] = 0
        import os
        self.path = ".dump/frame-"+self.prefix+'/'
        if not os.path.exists(self.path):
            os.makedirs(self.path)

    def postFrame(self):
        self.frame.calcParticleDirections()
        self.frame.calcMotionMatrix(self.reference)
        self.frame.cacheInfo(self.path+"frame"+str(self.i)+".dump")

        for k in self.edges.keys():
            self.edges[k] -= self.frame.deviation(k[0], k[1])
        super(ConnectionCalcHooker, self).postFrame()

class GuideHairHooker(Hooker):
    def __init__(self, reference, prefix, number):
