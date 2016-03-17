import cPickle as pickle
import metis_graph as mg
import matplotlib.pyplot as plt
import guide_hair as gh
import crash_on_ipy

n_step = 100

particle_graph = pickle.load(file('mgB.test'))
minVal = min(particle_graph.eweights)
maxVal = max(particle_graph.eweights)
interval = maxVal - minVal

print minVal, maxVal

for i in range(len(particle_graph.eweights)):
    particle_graph.eweights[i] = \
        int((particle_graph.eweights[i] - minVal) / interval * n_step)

import pymetis
_g = particle_graph
cut, vers = pymetis.part_graph(20, xadj=_g.xadj, adjncy=_g.adjncy, eweights=_g.eweights)

guideHair = gh.GroupedGraph(particle_graph, vers)
guideHair.solve()
