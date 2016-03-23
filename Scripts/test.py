# -*- coding: utf-8 -*-
# pa = [0, 2, 4, 6]
# pb = [1,2,0,2,0,1]
#
import numpy as np
from scipy.optimize import minimize
#
# def rosen(x):
#      """The Rosenbrock function"""
#      return sum(100.0*(x[1:]-x[:-1]**2.0)**2.0 + (1-x[:-1])**2.0)
#
# x0 = np.array([1.3, 0.7, 0.8, 1.9, 1.2])
# res = minimize(rosen, x0, method='nelder-mead',\
#         options={'xtol': 1e-8, 'disp': True})


# minimize
def func(x, sign=1.0):
    """ Objective function """
    return np.array([sign*(x[0]**2+2*x[0]+1+x[1]**2+2*x[1])])

def func_deriv(x, sign=1.0):
    """ Derivative of objective function """
    return np.array([2*x[0]+2, 2*x[1]+2])

def c(x):
    return x

cons = ({'type': 'ineq',
         'fun' : c,
         'jac' : lambda x: np.identity(2)
         })

res = minimize(func, [5.0, 100.0], args=(), jac=func_deriv,
               method='SLSQP', options={'disp': True}, constraints=cons)

print(res.x)
