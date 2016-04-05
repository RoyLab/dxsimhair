import nCache
import nCacheHooker as ch
from pipeline import *

class ConverterHooker(ch.Hooker):
    def __init__(self, fileName):
        super(ConverterHooker, self).__init__()
        self.file = open(fileName, 'w')
        if not self.file:
            raise Exception("File not open!")

    def postFrame(self):
        if self.i == 0:
            self.file.write(str(self.nFrame))
            self.file.write("\n")
            self.file.write(str(self.frame.n_particle))
            self.file.write("\n")

        self.file.write("Frame %d\n" % self.i)

        for pos in self.frame.data:
            self.file.write("%f %f %f\n" % (pos[0], pos[1], pos[2]))
        super(ConverterHooker, self).postFrame()

    def endLoop(self):
        self.file.close()
        self.bar.finish()

if __name__ == "__main__":
    conv = ConverterHooker("D:/data.dump")
    conv.startLoop("Measure the deviation:")
    nCache.loop(file2, conv, 200)
    conv.endLoop()
