import cPickle as pickle
import metis_graph as mg
import matplotlib.pyplot as plt
import guide_hair as gh
import crash_on_ipy
import mcimport
from progressbar import *
import numpy as np
import weight_estimate as wet

n_step = 100
n_group = 50

np.set_printoptions(suppress=True)

particle_graph = pickle.load(file('mgB.test'))
minVal = min(particle_graph.eweights)
maxVal = max(particle_graph.eweights)
interval = maxVal - minVal

print "low %.3f, high %.3f"%(minVal, maxVal)

for i in range(len(particle_graph.eweights)):
    particle_graph.eweights[i] = \
        int((particle_graph.eweights[i] - minVal + 0.01) / interval * n_step)

import pymetis
_g = particle_graph
cut, vers = pymetis.part_graph(n_group, xadj=_g.xadj, adjncy=_g.adjncy, eweights=_g.eweights)

for i in range(n_group):
    print "group %d: %d strands"%(i, vers.count(i))

hairGroup = gh.GroupedGraph(particle_graph, vers)
hairGroup.solve()

frames = mcimport.importFile("../../maya cache/03074/hair_nRigidShape1.xml")

count = 0
print "computing motion matrix of guides..."
pbar = ProgressBar().start()
for frame in frames:
    frame.loadCache(file(".dump/frame"+str(count)+".dump", 'rb'))
    frame.calcSelectedParticleMotionMatrices(frames[0], hairGroup.guide)
    pbar.update(((count/(len(frames)-1.0))*100))
    count += 1
pbar.finish()

model = wet.SkinModel(frames, hairGroup)
model.estimate()
model.dump(file(".dump/weights.dump", 'wb'))
