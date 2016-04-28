from frame import Frame
from progressbar import *
from GraphBuilder import *

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
    def __init__(self, guide, ref, prefix):
        super(GuideHairHooker, self).__init__()
        self.data = []
        self.guide = guide
        self.refFrame = ref
        self.prefix = prefix

    def postFrame(self):
        dumpFile = ".dump/frame-"+self.prefix+"/frame"+str(self.i)+".dump"
        self.frame.selectGuideHair(self.guide, dumpFile)
        self.frame.calcSelectedParticleMotionMatrices(self.refFrame, self.guide)
        self.frame.clearAsGuideInfo()
        self.data.append(self.frame)
        super(GuideHairHooker, self).postFrame()

    def getResult(self):
        return self.data

    def export(self, fileName, factor):
        f = open(fileName, "wb")
        import struct
        f.write(struct.pack('i', len(self.guide)))
        f.write(struct.pack('i', factor))
        f.write(struct.pack('i', self.nFrame))

        for idx in self.guide:
            f.write(struct.pack('i', idx))

        for i in range(self.nFrame):
            f.write(struct.pack('i', i))
            for idx in self.guide:
                trans = self.data[i].particle_motions[idx]
                for k in range(factor):
                    R, T = trans[k]
                    for a in range(3):
                        for b in range(3):
                            f.write(struct.pack('f', R[a, b]))
                    for a in range(3):
                        f.write(struct.pack('f', T[a]))

        f.close()
        return

class NormalHairHooker(Hooker):
    def __init__(self, guideData, ref, prefix, i, split, graph):
        super(NormalHairHooker, self).__init__()

        self.guide = guideData
        self.graph = graph
        self.prefix = prefix
        self.refFrame = ref

        self.data = []

        nStrand = self.guide[0].n_hair
        step = nStrand / split

        self.start = i * step
        self.end = (i+1) * step
        if self.end >= nStrand:
            self.end = nStrand

    def postFrame(self):
        dumpFile = ".dump/frame-"+self.prefix+"/frame"+str(self.i)+".dump"
        self.frame.selectNormalHair(self.start, self.end, dumpFile)
        self.data.append(self.frame)
        super(NormalHairHooker, self).postFrame()

    def endLoop(self):
        super(NormalHairHooker, self).endLoop()
        import weight_estimate as wet
        model = wet.SkinModel(self.refFrame, self.guide, self.data, self.graph, range(self.start, self.end))
        model.solve()
        self.weights = model.weights[self.start:self.end]
        self.error0 = model.error0
        self.error = model.error

    def getResult(self):
        return self.weights, self.error0, self.error
