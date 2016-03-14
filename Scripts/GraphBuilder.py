import igraph
from scipy.spatial import cKDTree
from mcimport import *
from progressbar import *
import ipdb

radius = 0.02
weak_coef = 0.3

def createKDTree(n_pts, data):
    tripples = arrayToTrippleList(data.data)
    kdt = cKDTree(tripples)
    return kdt

def arrayToTrippleList(arr):
    res = []
    for i in range(len(arr)/3):
        res.append((arr[3*i], arr[3*i+1], arr[3*i+2]))
    return res

# @profile
def createInitGraph(frames):
    n_pts = frames[0].n_particle
    n_frames = len(frames)

    edgeHash = {}
    pbar = ProgressBar().start()
    for i in range(n_frames):
        kdt = createKDTree(n_pts, frames[i%5])
        pairs = kdt.query_pairs(radius)
        pbar.update(((i/(n_frames-1.0))*100))
        if i == 0:
            edgeHash = dict.fromkeys(pairs, 1)
            continue
        for pair in pairs:
            edgeHash.setdefault(pair, 0)
            edgeHash[pair] += 1

    pbar.finish()
    return edgeHash

def filterEdges(edges, thresh):
    trash = []
    for key in edges:
        if edges[key] < thresh:
            trash.append(key)

    for key in trash:
        del edges[key]

    return edges

data = importFile("../../maya cache/03074/hair_nRigidShape1.xml")
graph = createInitGraph(data)
n_weak_thresh = len(data) * weak_coef
filterEdges(graph, n_weak_thresh)
