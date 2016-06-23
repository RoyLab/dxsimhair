
file1 = "E:/cache/329.xml"
file2 = "../../maya cache/03074/hair_nRigidShape1.xml"
file3 = "E:/c0418.xml"
# file3 = "D:/424.xml"

if __name__ == "__main__":

    import time
    import sys

    starttime = time.strftime('%Y-%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))
    print "start at:", starttime

    import crash_on_ipy
    import numpy as np
    np.set_printoptions(suppress=True)

    # parameter begin
    nFrame = 200
    nStep = 1000 # weight discretization
    nGroup = 200
    radius = 0.03
    frameFilter = 0.2
    prefix = ["s15000new2"]
    fileName = file3
    split=40
    guideOpts = ["rand", "opt", "worst"]
    bReport = True
    bMail = True
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
    import os

    def setReadOnly(fileName):
        os.system('attrib +r \"'+fileName+'\"')

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
        ruler.endLoop()

        pkl.dump(edges, file(prefix[0]+'-edges.dump', 'w'))
        setReadOnly(prefix[0]+'-edges.dump')

        #step 2
        strandGraph = gb.shrinkGraph(edges, factor)

        particleGraph = mg.MetisGraph(edges, nParticle)
        strandGraph = mg.MetisGraph(strandGraph, nStrand)
        gb.normalize(particleGraph, nStep)
        gb.normalize(strandGraph, nStep)

        pkl.dump(particleGraph, file(prefix[0]+'mgA.dump', 'w'))
        setReadOnly(prefix[0]+'mgA.dump')
        pkl.dump(strandGraph, file(prefix[0]+'mgB.dump', 'w'))
        setReadOnly(prefix[0]+'mgB.dump')

        pkl.dump((nStrand, nParticle, factor, refFrame, radius, frameFilter), file(prefix[0]+'info.dump', 'w'))
    else:
        nStrand, nParticle, factor, refFrame, radius, frameFilter = pkl.load(file(prefix[0]+'info.dump', 'r'))
        strandGraph = pkl.load(file(prefix[0]+'mgB.dump'))

        # step 3
        _g = strandGraph
        import pymetis
        cut, vers = pymetis.part_graph(nGroup, xadj=_g.xadj, adjncy=_g.adjncy, eweights=_g.eweights)

        f = open(prefix[0]+".group","wb")
        import struct
        f.write(struct.pack('i', len(vers)))
        for i in vers:
            f.write(struct.pack('i', i))
        f.close()

        # rand, opt, worst
        for iiii in range(3):
            opt = guideOpts[iiii]

            import guide_hair as gh
            starttime = time.strftime('%Y-%m-%d  %Hh%Mm%Ss',time.localtime(time.time()))
            hairGroup = gh.GroupedGraph(strandGraph, vers)
            hairGroup.solve(opt)
            sign = prefix[0] + '-'+opt+'-'
            sign += time.strftime('%m-%d %Hh%Mm%Ss',time.localtime(time.time()))

            guideImporter = ch.GuideHairHooker(hairGroup.guide, refFrame, prefix[0])
            guideImporter.startLoop("Import guide hair data with %d frames:" % nFrame)
            nCache.loop(fileName, guideImporter, nFrame)
            guideImporter.endLoop()

            guideData = guideImporter.getResult()
            guideExportFileName = sign+".guide"
            guideImporter.export(guideExportFileName, factor)
            setReadOnly(guideExportFileName)

            error0 = 0.0
            error = 0.0
            weights = []
            for i in range(split+1):
                nImporter = ch.NormalHairHooker(guideData, refFrame, prefix[0], i, split, hairGroup)
                nImporter.startLoop("precomputation %d / %d:" % (i+1, split))
                nCache.loop(fileName, nImporter, nFrame)
                nImporter.endLoop()
                w, e0, e = nImporter.getResult()
                weights += w
                error0 += e0
                error += e

            pkl.dump((hairGroup.guide, weights), file(sign+"-weights.dump", 'wb'), 2)
            setReadOnly(sign+"-weights.dump")

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
                content += 'Frame number: %d\n' % nFrame
                content += 'Guide sum %f, energy from %f t0 %f\n' % (hairGroup.energy, error0, error)
                content += 'Guide hair selection:\n'
                content += repr(hairGroup.guide)

                print content

                f = open("./Report/"+sign+".txt", 'w');
                f.write(content)
                f.close()

                if bMail:
                    send_mail(mailto_list, 'Report in 30/March '+sign+'-'+prefix[0], content)
