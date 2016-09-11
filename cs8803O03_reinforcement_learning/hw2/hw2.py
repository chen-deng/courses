import numpy as np

p = 1.0
V = [0.0,0.0,13.9,0.0,4.4,11.7,0.0, 0]
R = [-3.1,7.3,2.6,8.9,-3.4,-1.6,3.9, 0]

V_upper = [V[i] for i in map(int, "0134567")]
V_lower = [V[i] for i in map(int, "0234567")]
R_upper = [R[i] for i in map(int, "024567")]
R_lower = [R[i] for i in map(int, "134567")]
N = len(V_lower)

kest = []

for k in range(1, N):
  est_u = sum(R_upper[:k]) + V_upper[k]
  est_l = sum(R_lower[:k]) + V_lower[k]
  kest.append(p * est_u + (1 - p) * est_l)

kest = np.array(kest)

coef = kest - np.roll(kest, 1)
coef = coef[::-1]

for r in np.roots(coef):
  print r
