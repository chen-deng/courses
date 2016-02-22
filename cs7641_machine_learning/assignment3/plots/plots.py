import matplotlib.pylab as pl
import numpy as np
from collections import defaultdict

def plotsa():
	"""
	Plots Simulated annealing example
	"""
	fig1 = pl.figure()
	data = np.loadtxt("simulatedannealing1.csv", skiprows=1, delimiter=",")
	fvals =  data[:, 0]
	nevals = range(len(fvals))
	pl.loglog(nevals, fvals, 'b-')
	pl.xlabel("function evaluations", fontsize=20)
	pl.ylabel("cost function value", fontsize=20)
	pl.ylim([50, 5000])
	ax = fig1.gca()
	ax.tick_params(axis='x', labelsize=16)
	ax.tick_params(axis='y', labelsize=16)
	pl.savefig("simulatedannealing1.png", bbox_inches='tight')

def plotga():
	"""
	Plots genetic algorithm
	"""
	fig1 = pl.figure()
	data = np.loadtxt("geneticalgorithm.csv", skiprows=1, delimiter=",")
	fvals =  data[:, 0]
	nevals = range(len(fvals))
	pl.loglog(nevals, fvals, 'b-')
	pl.xlabel("function evaluations", fontsize=20)
	pl.ylabel("cost function value", fontsize=20)
	pl.ylim([50, 3000])
	ax = fig1.gca()
	ax.tick_params(axis='x', labelsize=16)
	ax.tick_params(axis='y', labelsize=16)
	# pl.tight_layout()
	pl.savefig("geneticalgorithm.png", bbox_inches='tight')	

def extract(filename):
	"""
	Extracts data from provided filename for part 2
	"""
	runtimes = defaultdict(list)
	fvals = defaultdict(list)
	iterations = defaultdict(list)
	with open(filename, "r") as f:
		for line in f:
			algo, niter, runtime, fval = line.strip().split(",")
			iterations[algo].append(int(niter))
			runtimes[algo].append(float(runtime))
			fvals[algo].append(float(fval))


	return iterations, runtimes, fvals

def plot_part2(filename):
	"""
	Plots the result of count ones test
	"""
	fig1 = pl.figure()
	iterations, runtimes, fvals = extract(filename)
	algos = ["SA", "GA", "MIMIC"]
	iters_sa, iters_ga, iters_mimic = [np.array(iterations[a]) for a in algos]
	runtime_sa, runtime_ga, runtime_mimic = [np.array(runtimes[a]) for a in algos]
	fvals_sa, fvals_ga, fvals_mimic = [np.array(fvals[a]) for a in algos]

	plotfunc = getattr(pl, "loglog")
	plotfunc(runtime_sa, fvals_sa, "bs", mew=0)
	plotfunc(runtime_ga, fvals_ga, "gs", mew=0)
	plotfunc(runtime_mimic, fvals_mimic, "rs", mew=0)

	# plotfunc(iters_sa, fvals_sa/(runtime_sa * iters_sa), "bs", mew=0)
	# plotfunc(iters_ga, fvals_ga/(runtime_ga * iters_ga), "gs", mew=0)
	# plotfunc(iters_mimic, fvals_mimic/(runtime_mimic * iters_mimic), "rs", mew=0)

	pl.xlabel("Runtime (seconds)")
	pl.ylabel("Objective function value")
	pl.ylim([min(fvals_sa) / 2, max(fvals_mimic) * 2])
	pl.legend(["SA", "GA", "MIMIC"], loc=4)

	pl.savefig(filename.replace(".csv", ".png"), bbox_inches="tight") 
	
def main():
	# plotsa()
	# plotga()
	datafiles = ["CountOnesTest.csv", "FourPeaksTest.csv", "KnapsackTest.csv",
				 "TravelingSalesmanTest.csv", "RastriginTest.csv"]
	for df in datafiles:
		plot_part2(df)

if __name__ == '__main__':
	main()