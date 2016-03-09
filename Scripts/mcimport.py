import nCache

def hooker(name, sz, arr):
    print arr[0], sz

def importFile(fileName):
    nCache.importFile(fileName, hooker)
