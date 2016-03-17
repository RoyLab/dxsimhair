import igraph
from scipy.spatial import cKDTree
from mcimport import *
from progressbar import *

import crash_on_ipy
np.set_printoptions(suppress=True)


radius = 0.02
weak_coef = 0.3

def createKDTree(n_pts, data):
    # tripples = arrayToTrippleList(data.data)
    kdt = cKDTree(data.data)
    return kdt

# def arrayToTrippleList(arr):
#     res = []
#     for i in range(len(arr)/3):
#         res.append((arr[3*i], arr[3*i+1], arr[3*i+2]))
#     return res

class Edge:
    def __init__(self, number, rec=None):
        self.value = number
        if rec == None:
            rec = []
        self.records = rec

# @profile
def createInitGraph(frames):
    n_pts = frames[0].n_particle
    n_frames = len(frames)

    edgeHash = {}
    pbar = ProgressBar().start()
    for i in range(n_frames):
        frame = frames[i]
        frame.computeMotionMatrix(frames[0])
        kdt = createKDTree(n_pts, frames[i])
        pairs = kdt.query_pairs(radius)
        pbar.update(((i/(n_frames-1.0))*100))
        if i == 0:
            edgeHash = dict.fromkeys(pairs, None)
            for key in edgeHash.keys():
                edgeHash[key] = Edge(1, [0])
            continue

        for pair in pairs:
            edgeHash.setdefault(pair, Edge(0))
            edge = edgeHash[pair]
            edge.value += 1
            edge.records.append(i)

    pbar.finish()
    return edgeHash

def filterEdges(edges, thresh):
    trash = []
    for key in edges:
        if edges[key].value < thresh:
            trash.append(key)
    for key in trash:
        del edges[key]
    return edges

frames = importFile("../../maya cache/03074/hair_nRigidShape1.xml")
graph = createInitGraph(frames)
n_weak_thresh = len(frames) * weak_coef
filterEdges(graph, n_weak_thresh)

for k in graph.keys():
    edge = graph[k]
    edge.value = 0
    for fn in edge.records:
        edge.value -= frames[fn].deviation(k[0], k[1])


f = open("data.txt", 'w')
import cPickle as pkl
pkl.dump(graph, f)
f.close()

# keys = graph.keys()
# print len(graph[keys[253]].records)
