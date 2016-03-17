class MetisGraph:
    def __init__(self, mygraph = None, n_vertices = 0):
        if mygraph == None:
            self.xadj = []
            self.adjncy = []
            self.eweights = []
        else:
            self.convertFromMyGraph(mygraph, n_vertices)

    def convertFromMyGraph(self, graph, n_vertices):
        if n_vertices == None or n_vertices <= 0:
            raise Exception("number of vertices must be determined!")

        vstat = [None] * n_vertices
        for i in range(n_vertices):
            vstat[i] = []

        # key tuple has been confirmed from small to large
        keys = graph.keys()
        for key in keys:
            vstat[key[0]].append([key[1], graph[key]])
            vstat[key[1]].append([key[0], graph[key]])

        self.xadj = [None] * (n_vertices+1)
        self.xadj[0] = 0
        self.adjncy = []
        self.eweights = []

        for i in range(n_vertices):
            self.xadj[i+1] = self.xadj[i] + len(vstat[i])
            for edge in vstat[i]:
                self.adjncy.append(edge[0])
                self.eweights.append(edge[1])
