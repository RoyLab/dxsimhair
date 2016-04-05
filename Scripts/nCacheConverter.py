import nCache
import nCacheHooker as ch
from pipeline import *
import struct

class ConverterHooker(ch.Hooker):
    def __init__(self, fileName):
        super(ConverterHooker, self).__init__()
        self.file = open(fileName, 'w')
        self.fileb = open(fileName+'b', 'wb')

        if not self.file or not self.fileb:
            raise Exception("File not open!")

    def postFrame(self):
        if self.i == 0:
            self.file.write(str(self.nFrame))
            self.file.write("\n")
            self.file.write(str(self.frame.n_particle))
            self.file.write("\n")

            self.fileb.write(struct.pack('i', self.nFrame))
            self.fileb.write(struct.pack('i', self.frame.n_particle))

        self.file.write("Frame %d\n" % self.i)
        self.fileb.write(struct.pack('i', self.i))

        for pos in self.frame.data:
            self.file.write("%f %f %f\n" % (pos[0], pos[1], pos[2]))

        self.data.tofile(self.fileb)
        super(ConverterHooker, self).postFrame()

    def endLoop(self):
        self.file.close()
        self.fileb.close()
        super(ConverterHooker, self).endLoop()

    def dataHooker(self, name, sz, arr):
        self.data = arr
        super(ConverterHooker, self).dataHooker(name, sz, arr)

if __name__ == "__main__":
    conv = ConverterHooker("D:/data2.dump")
    conv.startLoop("Measure the deviation:")
    nCache.loop(file1, conv, 200)
    conv.endLoop()
