import numpy as np
from scipy.optimize import minimize


def test1(a, idx):
    count = 0
    small = 0
    for p in a:
        if p[1] and idx in p[1]:
            # print p[0][p[1].index(idx)]
            if p[0][p[1].index(idx)] < 1.0e-5:
                small += 1
            count += 1
    print small, '%', count

def test2(a):
    '''analyse'''
    count = 0
    small = 0
    for p in a:
        if p[0] != None:
            for k in p[0]:
                if k < 1.0e-5:
                    small += 1
                count += 1
    print small, '%', count

import os
# os.system('attrib +r test.py')


# edges = []
# edges.append((0, 1))
# edges.append((0, 2))
# edges.append((0, 3))
# edges.append((0, 4))
# edges.append((4, 5))
# edges.append((5, 6))
# edges.append((6, 7))
# edges.append((7, 8))
# edges.append((8, 9))
# edges.append((8, 10))
# edges.append((8, 11))
# edges.append((8, 12))
# edges.append((6, 13))
# edges.append((13, 14))
# edges.append((14, 15))
# edges.append((15, 16))
# edges.append((15, 17))
# edges.append((15, 18))

import networkx as nx
import metis
G = metis.example_networkx()
import ipdb; ipdb.set_trace()
(edgecuts, parts) = metis.part_graph(G, 5)
colors = ['red','blue','green', 'yellow', 'black']
for i, p in enumerate(parts):
    G.node[i]['color'] = colors[p]
nx.drawing.nx_pydot.write_dot(G, 'example.dot') # Requires pydot or pygraphviz
