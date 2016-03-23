from scipy.spatial import cKDTree
from mcimport import *
from progressbar import *

np.set_printoptions(suppress=True)

n_particle_per_strand = 25
radius = 0.02
weak_coef = 0.2

def createKDTree(n_pts, data):
    kdt = cKDTree(data.data)
    return kdt

class Edge:
    def __init__(self, number, rec=None):
        self.value = number
        if rec == None:
            rec = []
        self.records = rec

    def __lt__(self, other):
        return self.value < other.value

    def __le__(self, other):
        return self.value <= other.value

    def __eq__(self, other):
        return self.value == other.value

    def __ne__(self, other):
        return self.value != other.value

    def __gt__(self, other):
        return self.value > other.value

    def __ge__(self, other):
        return self.value >= other.value

# @profile
def createInitGraph(frames):
    n_pts = frames[0].n_particle
    n_frames = len(frames)

    edgeHash = {}
    pbar = ProgressBar().start()
    for i in range(n_frames):
        frame = frames[i]
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
    before = len(edges)
    for key in edges:
        if edges[key].value < thresh:
            trash.append(key)
    for key in trash:
        del edges[key]

    print "Filter edge from %d to %d!" %(before, len(edges))
    return edges

def shrinkGraph(graph):
    smaller = {}
    for key in graph.keys():
        newkey = (key[0]/n_particle_per_strand, key[1]/n_particle_per_strand)
        smaller.setdefault(newkey, 0.)
        smaller[newkey] += graph[key]
    return smaller
