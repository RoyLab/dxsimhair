if __name__ == "__main__":

    import time
    import sys

    starttime = time.strftime('%Y-%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))
    print "start at:", starttime

    import crash_on_ipy
    import numpy as np
    np.set_printoptions(suppress=True)

    file1 = "E:/cache/329.xml"
    file2 = "../../maya cache/03074/hair_nRigidShape1.xml"

    # parameter begin
    nFrame = 2
    xmlFile = file2
    nStep = 1000 # weight discretization
    nGroup = 200
    radius = 0.02
    frameFilter = 0.2
    prefix = ["test"]
    fileName = file2
    split=3
    opt = "worst"
    bReport = True
    bMail = False
    # parameter end

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
        gb.normalize(particleGraph, nStep)
        gb.normalize(strandGraph, nStep)

        pkl.dump(particleGraph, file(prefix[0]+'mgA.dump', 'w'))
        pkl.dump(strandGraph, file(prefix[0]+'mgB.dump', 'w'))

        pkl.dump((nStrand, nParticle, factor, refFrame, radius, frameFilter), file(prefix[0]+'info.dump', 'w'))
    else:
        nStrand, nParticle, factor, refFrame, radius, frameFilter = pkl.load(file(prefix[0]+'info.dump', 'r'))
        strandGraph = pkl.load(file(prefix[0]+'mgB.dump'))

        # step 3
        _g = strandGraph
        import pymetis
        cut, vers = pymetis.part_graph(nGroup, xadj=_g.xadj, adjncy=_g.adjncy, eweights=_g.eweights)

        # rand, opt, worst
        import guide_hair as gh
        hairGroup = gh.GroupedGraph(strandGraph, vers)
        hairGroup.solve(opt)

        sign = time.strftime('__%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))

        guideImporter = ch.GuideHairHooker(hairGroup.guide, refFrame, prefix[0])
        guideImporter.startLoop("Import guide hair data:")
        nCache.loop(fileName, guideImporter, nFrame)
        guideImporter.endLoop()

        guides = guideImporter.getResult()

        weights = []
        error0 = 0.0
        error = 0.0
        for i in range(split):
            nImporter = ch.NormalHairHooker(guides, refFrame, prefix[0], i, split, hairGroup)
            nImporter.startLoop("precomputation %d/%d:" % (i+1, split))
            nCache.loop(fileName, nImporter, nFrame)
            nImporter.endLoop()
            w, e0, e = nImporter.getResult()
            weights += w
            error0 += e0
            error += e

        pkl.dump(weights, file(".dump/"+prefix[0]+sign+"weights.dump", 'wb'), 2)
        print "Total: error decrease from %f to %f." % (error0, error)

        endtime = time.strftime('%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))
        print "end at:", endtime


        # generate the report
        if bReport:
            from sendMail import *
            content = 'Processing from '+starttime+' to '+endtime+'\n'
            content += 'Weight discretization: %d\n' % nStep
            content += 'Group number: %d\n' % nGroup
            content += 'Radius : %f\n' % radius
            content += 'Frame filter: %f\n' % frameFilter
            content += 'Prefix: %s\n' % prefix[0]
            content += 'Signature: %s\n' % sign
            content += 'Guide selection: %s\n' % opt
            content += 'Guide sum %f, energy from %f t0 %f\n' % (hairGroup.energy, error0, error)

            f = open("./Report/"+sign+".txt", 'w');
            f.write(content)
            f.close()

            if bMail:
                send_mail(mailto_list, 'Report in 30/March '+prefix, content)
