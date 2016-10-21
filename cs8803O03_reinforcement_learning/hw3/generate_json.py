import json
import numpy as np

N = 29
shape = (N + 1, N + 1)
T1 = np.zeros(shape)
T2 = np.zeros(shape)

for i in range(1, N):
    T1[i, i + 1] = 1.0
    T2[i, i - 1] = 1.0

# Terminal states
T1[0, 0] = T2[0, 0] = 1.0
T1[N, N] = T2[N, N] = 1.0

# Rewards
R1 = np.zeros(shape)
R2 = np.zeros(shape)

for i in range(1, N):
    R1[i, i + 1] = -2.0
    R2[i, i - 1] = -1.0

# Terminal rewards
R1[0, 0] = R2[0, 0] = 0.0
R1[N, N] = R2[N, N] = 0.0
R1[N - 1, N] = R2[N - 1, N] = 100000


states = []
for i in range(shape[0]):
    actions = []
    transitions1 = []
    tid = 0        

    for j in range(shape[1]):
        if T1[i, j] > 0:
            transitions1.append({
                    "id": tid, 
                    "to": j, 
                    "probability": round(T1[i, j], 6), 
                    "reward": round(R1[i, j], 6)
                })
            tid += 1

    if transitions1:
        actions.append({"id": 0, "transitions": transitions1})

    transitions2 = []
    tid = 0
    for j in range(shape[1]):
        if T2[i, j] > 0:
            transitions2.append({
                    "id": tid, 
                    "to": j, 
                    "probability": round(T2[i, j], 6),
                    "reward": round(R2[i, j], 6)
                })
            tid += 1

    if transitions2:
        actions.append({"id": 1, "transitions": transitions2})

    states.append({"id": i, "actions": actions})
mdp = {"gamma": 0.75, "states": states}
print json.dumps(mdp)

with open("mdp.json", "w") as jf:
    json.dump(mdp, jf)