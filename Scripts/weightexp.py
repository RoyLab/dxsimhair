import cPickle as pkl
import struct

fileName = ".dump/s400004-06  02h45m59sweights.dump"
needMerge = True
factor = 5
weights = pkl.load(file(fileName, 'rb'))

if needMerge:
    n = len(weights)
    nStrand = n/factor
    for i in range(n/factor):
        for j in range(1, factor):
            if weights[j*nStrand+i][0] != None:
                weights[i] = weights[j*nStrand+i]
    weights = weights[:nStrand]

nStrand = len(weights)

print "there is %d strands." % nStrand

fileb = open("s4000-04-06 02h45m59s.weights", 'wb')
fileb.write(struct.pack('i', nStrand))

guideCount = 0
for i in range(nStrand):
    if type(weights[i][0]) != 'list':
        fileb.write(struct.pack('i', 1))
        fileb.write(struct.pack('i', i))
        fileb.write(struct.pack('f', 1.0))
        guideCount += 1
    else:
        nGuides = len(weights[i][0])
        fileb.write(struct.pack('i', nGuides))
        for j in range(nGuides):
            fileb.write(struct.pack('i', weights[i][0][j]))
            fileb.write(struct.pack('f', weights[i][1][j]))

fileb.close()

print "%d are guide strands" % guideCount
