if __name__ == "__main__":

    import time

    starttime = time.strftime('%Y-%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))
    print "start at:", starttime

    import crash_on_ipy
    import numpy as np
    np.set_printoptions(suppress=True)

    file1 = "E:/cache/329.xml"
    file2 = "../../maya cache/03074/hair_nRigidShape1.xml"

    nFrame = 2
    xmlFile = file2
    nStep = 100 # weight discretization
    nGroup = 50
    radius = 0.02
    frameFilter = 0.2
    prefix = ["test"]
    fileName = file2
    split=3

    needLoad = False
    import getopt
    try:
        (opts, args) = getopt.getopt(sys.argv[1:], "f:")
    except getopt.error:
        print "No dump data specified"

    if len(opts) != 0:
        for o,a in opts:
            if o == "-f":
                needLoad = True
                prefix[0] = a

    import cPickle as pkl
    import nCache
    import nCacheHooker as ch
    import GraphBuilder as gb
    import metis_graph as mg

    if not needLoad:
        # step 1
        builder = ch.GraphBuildHooker(radius)
        builder.startLoop("Build InitGraph:")
        nCache.loop(fileName, builder, nFrame)
        builder.endLoop()

        nStrand, nParticle, edges, refFrame = builder.graph()
        factor = nParticle/nStrand

        thresh = nFrame * frameFilter
        gb.filterEdges(edges, thresh)

        refFrame.calcParticleDirections()
        ruler = ch.ConnectionCalcHooker(edges, refFrame, prefix[0])
        ruler.startLoop("Measure the deviation:")
        nCache.loop(fileName, ruler, nFrame)
        builder.endLoop()

        pkl.dump(edges, file(prefix[0]+'-edges.dump', 'w'))

        #step 2
        strandGraph = gb.shrinkGraph(edges, factor)

        particleGraph = mg.MetisGraph(edges, nParticle)
        strandGraph = mg.MetisGraph(strandGraph, nStrand)
        mg.normalize(particleGraph)
        mg.normalize(strandGraph)

        pkl.dump(particleGraph, file(prefix[0]+'mgA.dump', 'w'))
        pkl.dump(strandGraph, file(prefix[0]+'mgB.dump', 'w'))

    else:
        strandGraph = pkl.load(file(prefix[0]+'mgB.test'))

        # step 3
        _g = strandGraph
        cut, vers = pymetis.part_graph(nGroup, xadj=_g.xadj, adjncy=_g.adjncy, eweights=_g.eweights)

        # rand, opt, worst
        hairGroup = gh.GroupedGraph(strandGraph, vers)
        hairGroup.solve("rand")

        sign = time.strftime('__%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))

        guideImporter = ch.GuideHairHooker(hairGroup.guide)
        guideImporter.startLoop("Import guide hair data:")
        nCache.loop(fileName, guideImporter, nFrame)
        guideImporter.endLoop()

        guides, reference = guideImporter.getResult()

        weights = []
        for i in range(split):
            nImporter = ch.NormalHairHooker(guides, reference, i, split)
            nImporter.startLoop("estimate weight %d/%d:" % (i+1, split))
            nCache.loop(fileName, nImporter, nFrame)
            weights += nImporter.endLoop()

        pkl.dump(weights, file(".dump/"+prefix[0]+sign+"weights.dump", 'wb'), 2)

        endtime = time.strftime('%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))
        print "end at:", endtime
