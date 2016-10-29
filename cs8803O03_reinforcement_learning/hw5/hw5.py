import re
import string
import pdb

F, NF, IDK = "FIGHT", "NO FIGHT", "I DON'T KNOW"
DCT = {0: NF, 1: F}

def strtol(str_):
    sl = re.findall("\[[0-1,]*\]", str_)
    return sl

def trim_hypothesis(agmt, hset, true_label):
    if true_label == NF:
        for f, ef in enumerate(agmt):
            for p, ep in enumerate(agmt):
                if (ef == 1) and (ep == 0):
                    hset.discard((f, p))

    if true_label == F:
        for f, ef in enumerate(agmt):
            for p, ep in enumerate(agmt):
                if ef == ep == 1:
                    hset.discard((f, p))

            if ef == 0:
                for i, _ in enumerate(agmt):
                    hset.discard((f, i))

def predict(agmt, hset, stored):

    # pdb.set_trace()   
    if sum(agmt) == len(agmt):
        return NF

    if agmt in stored:
        return stored[agmt]

    if len(hset) == 0:
        return NF

    # If all possible fighters are absent, then no fight
    no_fighters = True
    for idx, patron in enumerate(agmt):
        if patron == 1:
            for fidx, _ in list(hset):
                if idx == fidx:                    
                    no_fighters = False
                    break
    if no_fighters:        
        return NF

    # If all possible peacemakers are present, then no fight
    all_peacemakers = True
    for _, pidx in list(hset):
        for idx, pat in enumerate(agmt):
            if (idx == pidx) and (pat == 0):
                all_peacemakers &= False
                break

    if all_peacemakers:
        return NF 

    # If all peacemakers are absent *and* all fighters are present, then fight 
    any_peacemaker = False
    all_fighters = True
    for fidx, pidx in list(hset):
        for idx, pat in enumerate(agmt):
            if (pat == 1) and (idx == pidx):
                any_peacemaker |= True
                break

            if (idx == fidx) and (pat == 0):
                all_fighters &= False
                break

    if all_fighters and (not any_peacemaker) :
        return F

    return IDK

# def main():
with open("data.txt", "r") as w:
    fc = w.read().splitlines()

tx = string.maketrans("{}", "[]")
fc = [e.translate(tx) for e in fc]
agmts = [tuple(map(int, e[1:-1].split(","))) for e in strtol(fc[0])]
labels = map(int, fc[1][1:-1].split(","))    

N = len(agmts[0])
r = range(N)
hset = set([(i, j) for i in r for j in r if i != j])
stored = dict()

predictions = []
for agmt, nlbl in zip(agmts, labels):
    lbl = DCT[nlbl]
    pdc = predict(agmt, hset.copy(), stored)
    predictions.append(pdc)

    if (pdc == IDK) and len(hset) > 1:        
        stored[agmt] = lbl                
        trim_hypothesis(agmt, hset, lbl)        

print ",".join(predictions)

with open("results.csv", "w") as ouf:
    for agmt, label, pred in zip(agmts, labels, predictions):
        ouf.write("%s,%s,%s\n," %(".".join(map(str,agmt)), label, pred))






