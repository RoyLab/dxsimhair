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