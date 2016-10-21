import numpy as np

MAX = 256
N = 16
xi = [77, 36, 253, -202, 221, 171, -72, -226, 151, -2, 243, -205, 234, 102, 77]
yi = [-246, 6, 162, -183, 249, 18, -187, 135, -123, 164, 56, -127, 202, 20, 49]
xm = 12
ym = 10
k = 59

xp = np.hstack((xi[:xm], [0], xi[xm:]))
yp = np.hstack((yi[:ym], [0], yi[ym:]))
bestX = bestY = 256
LInfinityDistance = 10000


for x in range(-MAX, MAX + 1):
    for y in range(-MAX, MAX + 1):
        xp[xm] = x
        yp[ym] = y
        if np.abs(np.median(xp) - np.median(yp)) == k:
            lid = np.max(np.abs(xp - yp))
            if lid < LInfinityDistance:
                LInfinityDistance = lid            
                bestX, bestY = x, y

                print "bestX=%d, bestY=%d, LInfinityDistance=%d" \
                    %(x, y, LInfinityDistance)
