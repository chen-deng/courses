import matplotlib.pylab as pl

def reward_per_round(B):
	N = len(B)
	return sum([(i + 1) * (1.0 - b) for i, b in enumerate(B)])/float(N)

def expected_reward_infty(bad_sides_):
	r = reward_per_round(sides_, bad_sides_)
	print "reward per round = %f" % r
	q = 1.0 - float(sum(bad_sides_))/float(sides_)
	return r * q / (1.0 - q)**2

def amount_evolution(B, num_rounds):
	N = len(B)
	p = 1.0 - float(sum(B))/float(N)
	q = 1.0 - p
	r = reward_per_round(B)
	print "p = %f, q = %f, r = %f" % (p, q, r)
	amts = [0]
	amt = 0
	for i in range(num_rounds):		
		amt += r - q * amt
		amts.append(amt)		
		print "i = %d, amt = %f" % (i, amt)
	return amts

if __name__ == '__main__':
	B = [int(s) for s in "10101110"]
	amts_ = amount_evolution(B, 20)
	pl.plot(amts_, "o")
	pl.xlim([-1, 20])
	pl.ylim([-1, 5])
	pl.show()
	# print expected_reward(len(B), B)


