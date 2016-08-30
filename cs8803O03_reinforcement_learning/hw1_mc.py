import random as rd
import numpy as np

def roll(N_):
	return rd.choice(range(1, N_ + 1))

def game(B_, max_rounds):
	# Roll the dice, get the value `n` of the face. 
	# If bit number `n` is 1 in B_, then end the game, else
	# add `n` to total earnings.
	max_sides = len(B)
	nr = 0	
	amounts = np.zeros((1, max_rounds), dtype=np.float)
	terminate = False
	while (nr < max_rounds) and (terminate is False):
		side = roll(max_sides)		
		if B_[side - 1] == 0:
			amounts[0, nr] = amounts[0, nr - 1] + side
		else:
			amounts[0, nr] = 0.0
			terminate = True
		nr += 1
	return amounts

if __name__ == '__main__':
	B = [int(i) for i in "10101110"]
	max_games = 10000
	max_rounds = 5
	exp_amounts = np.zeros((1, max_rounds))

	for _ in range(max_games):
		exp_amounts += game(B, max_rounds) / float(max_games)

print exp_amounts




