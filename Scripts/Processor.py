import cPickle as pickle
import metis_graph as mg
import matplotlib.pyplot as plt
import guide_hair as gh
import crash_on_ipy
import mcimport
from progressbar import *
import numpy as np
import weight_estimate as wet
import time

n_step = 100
n_group = 200
n_frame = 200
prefix = '329'

np.set_printoptions(suppress=True)

print "training start at:", \
	time.strftime('%Y-%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))
starttime = time.strftime('%Y-%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))

particle_graph = pickle.load(file(prefix+'mgB.test'))
minVal = min(particle_graph.eweights)
maxVal = max(particle_graph.eweights)
interval = maxVal - minVal

print "low %.3f, high %.3f"%(minVal, maxVal)

for i in range(len(particle_graph.eweights)):
    particle_graph.eweights[i] = \
        int((particle_graph.eweights[i] - minVal) / interval * n_step + 1)

import pymetis
_g = particle_graph
cut, vers = pymetis.part_graph(n_group, xadj=_g.xadj, adjncy=_g.adjncy, eweights=_g.eweights)

for i in range(n_group):
    print "group %d: %d strands"%(i, vers.count(i))

hairGroup = gh.GroupedGraph(particle_graph, vers)
hairGroup.solve()

frames = mcimport.importFile("E:/cache/"+prefix+".xml", n_frame)

count = 0
print "computing motion matrix of guides..."
pbar = ProgressBar().start()
for frame in frames:
    frame.loadCache(file(".dump/"+prefix+"frame"+str(count)+".dump", 'rb'))
    frame.calcSelectedParticleMotionMatrices(frames[0], hairGroup.guide)
    pbar.update((count/(len(frames)-1.0))*100)
    count += 1
pbar.finish()

model = wet.SkinModel(frames, hairGroup)
model.estimate()
model.dump(file(".dump/"+prefix+"weights2.dump", 'wb'))

print "training end at:", \
	time.strftime('%Y-%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))

# model.assessment()

endtime = time.strftime('%Y-%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))
from sendMail import *

content = 'graph generated!\nfrom '+starttime+' to '+endtime+'\n'
content += 'group %d, prefix %s\n' % (n_group, prefix)
content += 'guide sum %f, energy from %f t0 %f\n' % (hairGroup.energy, model.error0, model.error)
send_mail(mailto_list, 'Report in 30/March '+prefix, content)

