from GraphBuilder import *
import cPickle as pkl
import getopt
from progressbar import *
import crash_on_ipy

n_frame = 5

if __name__ == "__main__":

    np.set_printoptions(suppress=True)
    needLoad = False
    try:
        (opts, args) = getopt.getopt(sys.argv[1:], "f:")
    except getopt.error:
        print "No dump data specified"

    if len(opts) != 0:
        fileName = ""
        for o,a in opts:
            if o == "-f":
                needLoad = True
                fileName = a

    if not needLoad:
        print "No dump data specified"

    if needLoad:
        n_particle, graph = pkl.load(file(fileName, 'r'))
    else:
        frames = importFile("../../maya cache/03074/hair_nRigidShape1.xml", n_frame)
        graph = createInitGraph(frames)
        n_weak_thresh = len(frames) * weak_coef
        filterEdges(graph, n_weak_thresh)

        for k in graph.keys():
            graph[k] = 0

        print "compute motion matrix..."
        pbar = ProgressBar().start()
        for fn in range(len(frames)):
            frames[fn].computeMotionMatrix(frames[0])
            frames[fn].cacheInfo(file(".dump/frame"+str(fn)+".dump", 'wb'))
            for k in graph.keys():
                graph[k] -= frames[fn].deviation(k[0], k[1])
            if fn > 0:
                frames[fn].clearMotionMatrix()
            pbar.update(((fn/(len(frames)-1.0))*100))
        pbar.finish()
        frames[0].clearMotionMatrix()

        pkl.dump(graph, file('data.test', 'w'))
        n_particle = frames[0].n_particle

    particle_graph = shrinkGraph(graph)

    import metis_graph as mg

    mgraph = mg.MetisGraph(graph, n_particle)
    mpgraph = mg.MetisGraph(particle_graph, n_particle / n_particle_per_strand)

    pkl.dump(mgraph, file('mgA.test', 'w'))
    pkl.dump(mpgraph, file('mgB.test', 'w'))
