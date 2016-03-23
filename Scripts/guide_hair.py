import metis_graph as mg

class GroupedGraph(mg.MetisGraph):

    def __init__(self, mgraph, group):

        self.xadj = mgraph.xadj
        self.adjncy = mgraph.adjncy
        self.eweights = mgraph.eweights
        self.hairGroup = group

        self.n_group = max(group)+1
        self.n_strand = len(group)

    def initSolution(self):

        self.initGuideHair()
        self.createLookupTable()

    def createLookupTable(self):
        '''convert the group table into a query link list'''
        self.lookup = []
        for i in range(self.n_group):
            self.lookup.append([])

        for i in range(self.n_strand):
            self.lookup[self.hairGroup[i]].append(i)

    def initGuideHair(self):
        self.guide = [None] * self.n_group
        self.guideVals = [-1] * self.n_group

        for i in range(self.n_strand):
            s = sum(self.eweights[self.xadj[i]:self.xadj[i+1]])
            if s > self.guideVals[self.hairGroup[i]]:
                if s == 0 and self.xadj[i] == self.xadj[i+1]:
                    continue
                self.guideVals[self.hairGroup[i]] = s
                self.guide[self.hairGroup[i]] = i

        for i in range(self.n_group):
            if self.guide[i] == None:
                self.guide[i] = self.hairGroup.index(i)
            self.guideVals[i] = self.validSum(self.guide[i], self.guide)

        print "\ninit guides: "
        print "  ", self.guide[:15], "..."
        print "init values: "
        print "  ", self.guideVals[:15], "..."

    def computeEnergy(self, guide=None):
        if guide == None:
            guide = self.guide
        eitr = mg.UndirectedIterator(self)
        result = 0.
        for i, j, weight in eitr:
            if validPair(i, j, guide):
                result += weight
        return result

    def isGuideHair(self, id, guide=None):
        if guide == None:
            guide = self.guide
        groupId = self.hairGroup[id]
        return id == guide[groupId]

    def validPair(self, i, j, guide):
        return (isGuideHair(i, guide) and (not isGuideHair(j, guide))) or \
            (isGuideHair(j, guide) and (not isGuideHair(i, guide)))

    def validSum(self, i, guide):
        eitr = mg.EdgeIterator(self, i)
        result = 0
        for j, weight in eitr:
            if not self.isGuideHair(j, guide):
                result += weight
        return result

    def validSum1(self, i, guide):
        eitr = mg.EdgeIterator(self, i)
        result = 0
        for j, weight in eitr:
            if self.isGuideHair(j, guide):
                print "strand %d need iteration" % i
            if not self.isGuideHair(j, guide):
                result += weight
        return result

    def iterate(self):
        changed = False
        for groupId in range(self.n_group):
            sub = self.validSum(self.guide[groupId], self.guide)
            for curNode in self.lookup[groupId]:
                origin, self.guide[groupId] = self.guide[groupId], curNode
                add = self.validSum(curNode, self.guide)
                # print add, sub
                if add > sub:
                    changed = True
                    self.guideVals[groupId] -= sub
                    self.guideVals[groupId] += add
                    sub = add
                else:
                    self.guide[groupId] = origin
        return changed

    def solve(self):
        self.initSolution()
        count = 0
        while 1:
            count += 1
            if not self.iterate():
                break

        print "\niterators: %d" % count
        print "%d groups:" % len(self.guide)
        print "  ", self.guide[:15], "..."
        print "values: "
        print "  ", self.guideVals[:15], "..."

        return self.guide
