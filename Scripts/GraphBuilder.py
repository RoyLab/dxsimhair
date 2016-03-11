import igraph
from scipy.spatial import cKDTree
from mcimport import *
from progressbar import *
import ipdb

radius = 0.02
weak_coef = 0.2

def createKDTree(n_pts, data):
    tripples = arrayToTrippleList(data.data)
    kdt = cKDTree(tripples)
    return kdt

def arrayToTrippleList(arr):
    res = []
    for i in range(len(arr)/3):
        res.append((arr[3*i], arr[3*i+1], arr[3*i+2]))
    return res

data = importFile("../../maya cache/03074/hair_nRigidShape1.xml")
# graph = createInitGraph(data)
frames = data

# @profile
# def createInitGraph(frames):
ptGraph = igraph.Graph()
n_pts = frames[0].n_particle
n_frames = len(frames)
n_weak_thresh = n_frames * weak_coef

ptGraph.add_vertices(n_pts)
ptGraph.es["weight"] = 1.0

edgeHash = {}
pbar = ProgressBar().start()
for i in range(n_frames*20):
    kdt = createKDTree(n_pts, frames[i%5])
    pairs = kdt.query_pairs(radius)

    # print "process frame %d, with %d edges..." % (i, len(pairs))
    pbar.update(((i/(n_frames*20.0-1))*100))

    if i == 0:
        edgeHash = dict.fromkeys(pairs, 1)
        continue

    for pair in pairs:
        edgeHash.setdefault(pair, 0)
        edgeHash[pair] += 1

pbar.finish()

weakLinks = ptGraph.es.select(weight_lt = n_weak_thresh)
ptGraph.delete_edges(weakLinks)
gg = ptGraph
