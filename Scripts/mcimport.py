import nCache
import numpy as np
import ipdb
from rigid_transform import *

n_particle_per_strand = 25

def importFile(fileName):
    hk = Hooker()
    nCache.importFile(fileName, hk)
    return hk.get_data()

class Hooker:

    def __init__(self):
        self.frames = []
        self.cframe = None

    def data_hooker(self, name, sz, arr):
        # print sz, arr[0:5], len(arr), name
        self.cframe.loadIntoMemory(name, sz, arr)

    def new_frame(self):
        self.cframe = Frame()
        self.frames.append(self.cframe)

    def get_data(self):
        return self.frames


class Frame:

    def __init__(self):
        self.count = 0
        self.data = None
        self.headData = None

        self.n_headVertex = 0
        self.n_hair = 0
        self.n_particle = 0

        self.rigid_motion = None
        self.particle_motions = []
        self.reference = None

        self.hairspline = []
        self.particle_direction = None

    def loadIntoMemory(self, name, sz, data):
        if self.count == 0:
            self.n_headVertex = int(sz)
            self.headData = np.matrix(data)
            self.headData.resize(len(data)/3, 3)

        elif self.count == 1:
            self.n_hair = int(data[0])
            self.n_particle = self.n_hair * n_particle_per_strand

        elif self.count == 3:
            self.data = np.array(data)
            self.data.resize(self.n_particle, 3)

        self.count += 1

    # @profile
    def _computeParticleMatrices(self):
        from scipy import interpolate
        u_axis = np.linspace(0, 1, 25)

        directions = []
        for i in range(self.n_hair):
            begin = n_particle_per_strand*i
            end = n_particle_per_strand*(i+1)

            data = self.data[begin:end].T
            spline, u = interpolate.splprep(data, s=0)
            derive = interpolate.splev(u_axis, spline, der=1)

            derive = np.matrix(derive).T
            for j in range(n_particle_per_strand):
                directions.append(derive[j] / np.linalg.norm(derive[j]))

            self.hairspline.append(spline)

        self.particle_direction = np.array(directions)
        self.particle_direction.resize(self.n_particle, 3)

        ref = self.reference
        for i in range(self.n_particle):
            trans = self.data[i] - (ref.data[i] + self.rigid_motion[1])
            rot = vector_rotation_3D((ref.particle_direction[i] * self.rigid_motion[0].T).A1, self.particle_direction[i])
            self.particle_motions.append((rot, trans))

    def applyRigidTrans(vec, rot, trans):
        return vec * rot.T + trans

    def deviation(self, id0, id1):
        pos0 = self.data[id0]
        pos1 = self.data[id1]
        dir0 = self.particle_direction[id0]
        dir1 = self.particle_direction[id1]

        t0 = self.particle_motions[id0]
        t1 = self.particle_motions[id1]
        t = self.rigid_motion

        posr0 = self.reference.data[id0] + t[1]
        posr1 = self.reference.data[id1] + t[1]
        dirr0 = self.reference.particle_direction[id0] * t[0].T
        dirr1 = self.reference.particle_direction[id1] * t[0].T

        dp01 = pos0-(posr0+t1[1])
        dp10 = pos1-(posr1+t0[1])

        dr01 = dir0-(dirr0*t1[0].T).A1
        dr10 = dir1-(dirr1*t0[0].T).A1

        return dp01.dot(dp01) + dp10.dot(dp10) +\
            dr01.dot(dr01) + dr10.dot(dr10)

    def computeMotionMatrix(self, reference):
        self.reference = reference
        self.rigid_motion = rigid_transform_3D(reference.headData, self.headData)
        self._computeParticleMatrices();
