import random as rd
import numpy as np
from matplotlib import rc
import matplotlib.pylab as pl

# rc("text", usetex=True)

def roll(N_):
  return rd.choice(range(1, N_ + 1))

def is_bad(n, B_):
  return B_[n - 1] == 1

def game(B_, quit_amt):
  max_sides = len(B_) 
  tot_amt = 0
  while tot_amt <= quit_amt:
    a = roll(max_sides)
    if is_bad(a, B_):
      return 0
    tot_amt += a
  return tot_amt

if __name__ == '__main__':
  DIEN = "0111001100111011001011111001"
  MAX_GAMES = 10000000
  B = [int(i) for i in DIEN]  
  quitting_amounts = range(0, 15, 1)
  earnings = np.zeros((MAX_GAMES, len(quitting_amounts)))
  for idx, qamt in enumerate(quitting_amounts):
    exp_ern = []
    print "quitting amount = $%s" % qamt
    for _ in range(MAX_GAMES):            
      exp_ern.append(game(B, qamt))   
    earnings[:, idx] = exp_ern  
  
  earnings_mean = np.mean(earnings, axis=0)
  earnings_sd = 2.6 * np.std(earnings, axis=0) / np.sqrt(MAX_GAMES)

  pl.figure(figsize=(8, 8))
  pl.errorbar(quitting_amounts, earnings_mean, yerr=earnings_sd, 
              fmt="o", ecolor="b")
  pl.xlim([-2, max(quitting_amounts) + 1])
  pl.xlabel("quitting amount (USD)")
  pl.ylabel("expected earnings (USD)")
  ymin, ymax = min(earnings_mean) - 0.1, max(earnings_mean) + 0.1
  pl.ylim([ymin, ymax])
  pl.text(20, 0.9 * ymax, "B = {%s}" % DIEN, fontsize=9)
  pl.show()
  # pl.savefig("die_%s.png" % DIEN, bbox_inches="tight")   


