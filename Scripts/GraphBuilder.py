import igraph
from scipy.spatial import cKDTree
from mcimport import *
import ipdb

radius = 0.02
weak_coef = 0.2

def createInitGraph(frames):
    ptGraph = igraph.Graph()
    n_pts = frames[0].n_particle
    n_frames = len(frames)
    n_weak_thresh = n_frames * weak_coef

    ptGraph.add_vertices(n_pts)
    ptGraph.es["nearCount"] = None

    for i in range(n_frames):
        kdt = createKDTree(n_pts, frames[i])
        pairs = kdt.query_pairs(radius)

        print "process frame %d, with %d edges..." % (i, len(pairs))

        # ipdb.set_trace()
        # ids = ptGraph.add_edges(pairs)
        for pair in pairs:
            if not ptGraph.are_connected(pair[0], pair[1]):
                ptGraph.add_edge(pair[0], pair[1], nearCount = 1)
            else:
                eid = ptGraph.get_eid(pair[0], pair[1])
                ptGraph.es[eid]["nearCount"] += 1

    weakLinks = ptGraph.es.select(nearCount_lt = n_weak_thresh)
    ptGraph.delete_edges(weakLinks)


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
graph = createInitGraph(data)
