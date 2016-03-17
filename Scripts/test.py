# -*- coding: utf-8 -*-
pa = [0, 2, 4, 6]
pb = [1,2,0,2,0,1]

import pymetis

print pymetis.part_graph(2, xadj=pa, adjncy=pb)
# print pymetis.part_graph(3, adjacency={0:[1,2], 5:[0,2], 2:[0,1]})
