# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pylab as pl
from scipy import interpolate

x = np.linspace(0, 2*np.pi+np.pi/4, 10)
ysin1 = np.sin(x)
yrand1 = np.random.rand((10))
yfix3 = [[-0.81769186, -0.88824874, -0.92432022, -0.94022739, -0.97809589,\
  -0.98703551, -0.98917031, -0.99855345, -1.00039852, -1.00148332,\
  -1.00114572, -1.00162184, -1.00106525, -1.00030756, -0.9992401 ,\
  -0.9984197 , -0.99801761, -0.99837869, -0.99842077, -0.99860424,\
  -0.99892092, -0.99964339, -1.00026703, -1.00099218, -1.0018152 ],\
 [-0.28586382, -0.33036175, -0.41282433, -0.49410132, -0.5805558 ,\
  -0.67086774, -0.76129407, -0.85087031, -0.94109124, -1.03115606,\
  -1.12110829, -1.2109344 , -1.30062723, -1.39018106, -1.47959328,\
  -1.56886995, -1.6580112 , -1.74701416, -1.83587217, -1.92460048,\
  -2.0131793 , -2.10162783, -2.18993282, -2.27810407, -2.36614585],\
 [ 0.89791399,  0.93886626,  0.92813349,  0.97079885,  0.9787811 ,\
   0.96640074,  0.96785009,  0.97111022,  0.97165287,  0.97198677,\
   0.97256905,  0.97237879,  0.97251129,  0.97249204,  0.97240585,\
   0.97228491,  0.97191542,  0.97168845,  0.97045267,  0.9701553 ,\
   0.96868342,  0.96808362,  0.96683043,  0.96599603,  0.96544123]]

# from mpl_toolkits.mplot3d import Axes3D
# fig = pl.figure()
# ax = fig.add_subplot(111, projection='3d')
# fig.show()

x_new = np.linspace(0, 2*np.pi+np.pi/4, 100)
u_new = np.linspace(-0.0, 1.0, 1000)
y = yrand1
f_linear = interpolate.interp1d(x, y)
tck = interpolate.splrep(x, y, k=2)
tck1 = interpolate.splrep(x, y, k=3)
tck2 = interpolate.splrep(x, y, k=4)
tck3, u = interpolate.splprep([x, y], s=0, k = 3)
yb = interpolate.splev(x_new, tck)
yb1 = interpolate.splev(x_new, tck1)
yb2 = interpolate.splev(x_new, tck2)
yb3 = interpolate.splev(u_new, tck3)

# pl.plot(yb[0], yb[1], yb[2], label=u"线性插值")
# pl.plot(y1[0], y1[1], y1[2], label="2")
pl.plot(x_new, f_linear(x_new), label=u"1")
pl.plot(x_new, yb, label=u"B-spline2")
pl.plot(x_new, yb1, label=u"B-spline3")
pl.plot(x_new, yb2, label=u"B-spline4")
pl.plot(yb3[0], yb3[1], label=u"B-spline4")
# pl.plot(y_bspline1[0], y_bspline1[1], label=u"B-spline插值1")
# pl.xlim(-1,10)
# pl.ylim(-1.1, 1.1)
pl.legend()
pl.show()

#
# import matplotlib.pyplot as plt
