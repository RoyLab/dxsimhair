from weight_estimate import npar
from struct import pack
import coordinates as cd
import numpy as np
import array

class HairInterpolation:

    def __init__(self, guideData, weights, reference, fileName):
        self.guideData = guideData
        self.weights = weights
        self.refFrame = reference
        self.fileName = fileName

        self.nFrame = len(guideData)
        self.nStrand = len(weights)

        print "frame %d, strand %d" % (self.nFrame, self.nStrand)

    def interpolate(self):
        f = open(self.fileName, 'wb')
        f.write(pack('i', self.nFrame))
        f.write(pack('i', self.nStrand*npar))

        bar = ProgressBar().start()
        for i in range(self.nFrame):
            bar.update(i*100/self.nFrame)
            f.write(pack('i', i))
            for j in range(self.nStrand):
                s = self.strandInterpolation(j, i)
                for k in range(npar*3):
                    f.write(pack('f', s[k]))
        bar.finish()
        f.close()

    def strandInterpolation(self, s, fn):
        if self.weights[s][0] == None:
            Ci = [s]
            w = 1.0
        else:
            Ci = self.weights[s][1]
            w = self.weights[s][0]

        t0 = self.refFrame.data[s*npar:(s+1)*npar]
        guide = self.guideData[fn]
        tref = cd.rigid_trans_batch_no_dir(guide.rigid_motion, t0)
        A = []
        for g in Ci:
            Bg = guide.particle_motions[g]
            state = np.array(cd.point_trans_batch_no_dir(Bg, tref))
            import ipdb; ipdb.set_trace()
            state.resize(3*npar)
            A.append(state)
        A = np.matrix(A)
        return (A.T * np.matrix(w).T).A1

class HairInterpolation20:

    def __init__(self, guideData, weights, reference, fileName):
        self.guideData = guideData
        self.weights = weights
        self.refFrame = reference
        self.fileName = fileName

        self.nFrame = len(guideData)
        self.nStrand = len(weights)

        print "frame %d, strand %d" % (self.nFrame, self.nStrand)

    # @profile
    def interpolate(self):
        f = open(self.fileName, 'wb')
        f.write(pack('i', self.nFrame))
        f.write(pack('i', self.nStrand*npar))

        bar = ProgressBar().start()
        for i in range(self.nFrame):
            bar.update(i*100/self.nFrame)
            f.write(pack('i', i))
            for i1 in range(4):
                for j in range(4):
                    f.write(pack('f', self.guideData[i].rigidMotionMatrix[i1, j]))

            poss = np.empty(self.nStrand*npar*3)
            dirs = np.empty(self.nStrand*npar*3)
            for j in range(self.nStrand):
                s = self.strandInterpolation(j, i)
                poss[j*npar*3:(j+1)*npar*3] = s[:npar*3]
                dirs[j*npar*3:(j+1)*npar*3] = s[npar*3:]
                # import ipdb; ipdb.set_trace()

            array.array('f', poss).tofile(f)
            array.array('f', dirs).tofile(f)

        bar.finish()
        f.close()

    # @profile
    def strandInterpolation(self, s, fn):
        if self.weights[s][0] == None:
            Ci = [s]
            w = 1.0
        else:
            Ci = self.weights[s][1]
            w = self.weights[s][0]

        s0 = (self.refFrame.data[s*npar:(s+1)*npar], self.refFrame.particle_direction[s*npar:(s+1)*npar])
        guide = self.guideData[fn]
        tref = cd.rigid_trans_batch(guide.rigid_motion, s0)
        A = []
        for g in Ci:
            Bg = guide.particle_motions[g]
            state = np.array(cd.point_trans_batch(Bg, tref))
            state.resize(6*npar)
            A.append(state)
        A = np.matrix(A)
        return (A.T * np.matrix(w).T).A1

if __name__ == "__main__":
    import cPickle as pkl
    import nCacheHooker as ch
    import nCache
    from progressbar import*

    nFrame = 200
    fileName = "../../maya cache/03074/hair_nRigidShape1.xml"
    prefix = "s4000"

    # random
    wFile = ".dump/s400004-06  10h22m06sweights.dump"
    cacheFile = "s4000-04-06 10h22m06s.anim"
    guide = [1964, 206, 775, 568, 1534, 708, 1606, 642, 1418, 1364, 636, 990, 1244, 373, 1777, 918, 712, 439, 1584, 2008, 1467, 557, 1232, 235, 492, 188, 357, 85, 2322, 687, 564, 909, 1414, 839, 3939, 1531, 80, 317, 93, 994, 1051, 910, 1601, 861, 657, 385, 1915, 1530, 1540, 1425, 246, 1426, 1969, 1367, 793, 2013, 1977, 1588, 1876, 1585, 321, 2070, 1821, 796, 1068, 2114, 1818, 713, 2299, 1933, 1667, 1004, 1495, 936, 801, 175, 133, 3193, 43, 1204, 946, 1997, 1839, 2046, 1445, 398, 407, 1272, 1845, 1270, 1948, 1844, 1949, 3162, 734, 3910, 3756, 3539, 2088, 1726, 1611, 2082, 4062, 1895, 2591, 945, 541, 1559, 4113, 2467, 1993, 1380, 749, 2, 9, 179, 224, 277, 615, 1026, 1338, 1216, 1275, 1394, 1849, 1742, 747, 1901, 1150, 225, 410, 111, 1697, 4127, 1223, 1219, 1032, 1282, 1399, 1158, 1036, 1285, 623, 420, 757, 1099, 969, 830, 1544, 833, 3884, 3405, 3893, 1006, 3137, 871, 3199, 3329, 3398, 1502, 3084, 3757, 2269, 2648, 3607, 3007, 1782, 3643, 4015, 3892, 3186, 3600, 3077, 668, 3204, 3859, 217, 3696, 3820, 3134, 2764, 3478, 2658, 2327, 3620, 3274, 3098, 4070, 2446, 3159, 2141, 2975, 3287, 3978, 3533, 3567, 4228, 3635, 4239, 3913, 3564, 2788, 2727, 3029, 2496, 3841, 2939, 4149, 3833, 2960, 3695, 3771, 3496, 2234, 2135, 3090, 3349, 3400, 2666, 3281, 2963, 3150, 2892, 3052, 4208, 3250, 2750, 2749, 3182, 3595, 3119, 2143, 3593, 3055, 2068, 2302, 3798, 61, 2063, 1122, 2395, 2627, 2292, 3061, 3458, 2878, 2879, 2123, 3752, 2468, 1884, 1199, 1011, 1200, 2363, 2173, 2170, 3741, 166, 3189, 2211, 1946, 3051, 3942, 2342, 3039, 4188, 3499, 2500, 3646, 3922, 2988, 2920, 2798, 3647, 3305, 4212, 3300, 3648, 3759, 4159, 3456, 3926, 2745, 3157, 3224, 3375, 2930, 2509, 3801, 4048, 3046, 4126, 3995, 3655, 3865, 3584, 2827, 4133, 2861]

    # worst
    # wFile = ".dump/s400004-06  02h45m59sweights.dump"
    # cacheFile = "s4000-worst-04-06 02h45m59.anim"
    #
    # guide = [114, 196, 501, 502, 304, 2107, 21, 59, 58, 203, 3, 63, 195, 307, 1777, 242, 306, 243, 1962, 283, 1751, 904, 149, 187, 359, 188, 1528, 365, 3885, 348, 240, 116, 1300, 178, 3226, 912, 57, 201, 8, 1370, 570, 38, 126, 202, 588, 256, 36, 2201, 648, 2023, 2248, 448, 1310, 717, 855, 1124, 2102, 64, 1197, 639, 1982, 2070, 87, 998, 453, 2112, 2153, 1880, 2351, 2405, 90, 800, 1973, 936, 2198, 46, 23, 214, 42, 464, 811, 1786, 138, 132, 183, 274, 171, 1510, 472, 411, 1081, 226, 1570, 3569, 169, 4172, 2151, 3811, 165, 3530, 2116, 2084, 3809, 66, 3949, 399, 541, 538, 2183, 392, 170, 97, 11, 0, 9, 50, 10, 137, 1517, 1279, 347, 1513, 1630, 227, 887, 130, 118, 1273, 544, 225, 100, 55, 7, 2200, 185, 962, 113, 473, 826, 1158, 228, 17, 416, 412, 963, 1286, 1035, 1102, 2142, 1037, 4169, 2855, 3953, 3980, 4057, 3677, 4009, 4143, 3398, 131, 4171, 3819, 3879, 4139, 4010, 68, 96, 3260, 3747, 3606, 4006, 3600, 3952, 262, 4103, 2334, 327, 3164, 3820, 4100, 4102, 4106, 2325, 3886, 3619, 2231, 3948, 2233, 2012, 3954, 4008, 4101, 4104, 2557, 4058, 3710, 3095, 3158, 332, 3497, 2229, 2279, 2139, 2495, 2331, 3154, 3678, 4202, 2223, 4200, 2381, 2230, 3246, 2434, 4059, 2280, 2383, 3679, 3153, 2493, 3206, 3621, 3887, 3662, 2384, 3384, 3457, 2575, 3944, 2291, 2348, 2158, 2693, 3737, 2251, 2302, 2203, 61, 2459, 2462, 2461, 2635, 2300, 463, 3943, 128, 3745, 2359, 3752, 3601, 172, 596, 168, 3393, 461, 3673, 3877, 325, 166, 3945, 394, 884, 3661, 2634, 3500, 3917, 3644, 3499, 3165, 3502, 3715, 3117, 3385, 3576, 2737, 3598, 3108, 3930, 3651, 4170, 3432, 2632, 3498, 3317, 2499, 3755, 2520, 3515, 2568, 4064, 3589, 3730, 3511, 3512, 3653, 3514, 3307, 3823, 3657, 3359]

    # opt
    # wFile = ".dump/s400004-05  20h59m46sweights.dump"
    # cacheFile = "s4000-opt-04-05 20h59m46s.anim2"
    #
    # guide = [908, 1483, 1759, 1353, 1704, 1822, 1486, 1587, 1108, 1708, 1291, 920, 1244, 1865, 2058, 1054, 1965, 1473, 2094, 2052, 1908, 557, 970, 1288, 1229, 698, 1470, 1101, 3618, 429, 838, 907, 1862, 839, 2725, 905, 843, 1309, 790, 259, 773, 694, 1249, 1252, 1662, 1372, 2053, 2054, 1481, 1428, 2097, 1426, 1868, 1764, 1059, 1247, 323, 1350, 1924, 1121, 1971, 1193, 1812, 1313, 2015, 1882, 1543, 1107, 525, 2021, 1480, 934, 390, 1605, 731, 1678, 673, 1724, 3067, 1615, 1080, 1509, 1505, 1014, 1741, 1201, 1567, 1736, 1679, 1208, 1947, 1144, 1564, 3493, 1008, 3910, 3548, 2427, 2030, 1937, 1984, 1496, 3828, 1895, 4181, 876, 1788, 1991, 3347, 2467, 1500, 1140, 1343, 53, 1625, 180, 964, 1675, 1688, 1846, 891, 1456, 1791, 1571, 1215, 1850, 1427, 1683, 1569, 1629, 1333, 1747, 1349, 4232, 1347, 1685, 1578, 1744, 1460, 421, 968, 1285, 1696, 1579, 1635, 1906, 1039, 1580, 1656, 1346, 2711, 3279, 3758, 4233, 2372, 2996, 2370, 3144, 2538, 1563, 3839, 3614, 2476, 2705, 3688, 1441, 1725, 3701, 2714, 3971, 4082, 2814, 3015, 2941, 2833, 3859, 1137, 2985, 2944, 3207, 4121, 3478, 4024, 3024, 2603, 2661, 3577, 4118, 2446, 2424, 2911, 3064, 2712, 4209, 2599, 3284, 4210, 3491, 4240, 3351, 3285, 3772, 4186, 4157, 3421, 4158, 1383, 3280, 4073, 4081, 2779, 3213, 3496, 3764, 3482, 2843, 3689, 2528, 4156, 3560, 4074, 3087, 3555, 4168, 2784, 2628, 3941, 2749, 2508, 2288, 3875, 3521, 2991, 3054, 1547, 1432, 4194, 763, 3112, 2057, 4192, 2343, 2511, 2261, 2755, 2470, 4123, 2997, 4125, 2307, 1884, 1890, 1837, 1200, 2363, 1842, 3003, 1613, 2581, 3189, 2883, 3130, 3861, 2623, 4167, 4243, 4188, 4184, 4041, 3168, 4230, 4050, 3364, 4086, 3990, 3305, 4215, 4162, 4213, 3025, 4159, 2236, 4122, 4097, 4244, 3149, 4051, 4136, 4095, 4183, 4166, 4195, 4126, 4085, 4231, 2566, 2185, 3214, 4190, 2445]

    nStrand, nParticle, factor, refFrame, radius, frameFilter = pkl.load(file(prefix+'info.dump', 'r'))
    weights = pkl.load(file(wFile, 'rb'))
    factor = 5

    n = len(weights)
    print len(weights)
    if n != 4245:
        nStrand = n/factor
        for i in range(n/factor):
            for j in range(1, factor):
                if weights[j*nStrand+i][0] != None:
                    weights[i] = weights[j*nStrand+i]
        weights = weights[:nStrand]
    else:
        nStrand = n

    print len(weights)

    guideImporter = ch.GuideHairHooker(guide, refFrame, prefix)
    guideImporter.startLoop("Import guide hair data with %d frames:" % nFrame)
    nCache.loop(fileName, guideImporter, nFrame)
    guideImporter.endLoop()

    guideData = guideImporter.getResult()

    cache = HairInterpolation20(guideData, weights, refFrame, cacheFile)
    cache.interpolate()
