import re
import sys
import matplotlib.pylab as pl
import numpy as np

MARKER_SIZE=5
MARKER_EDGE_WIDTH=1

def parse_output(file_name):
	"""
	Output parser function: takes location of 
	"""

	lines = open(file_name).read().splitlines()

	# Patterns
	input_pat = "RUN_FIXED\((\d+), (FIXED_SIZE|FIXED_FILE), (\d+)\)"
	throughput_pat = "Throughput\s*=\s*([\d\.]+) bytes/sec"
	respt_pat="Avg. Response Time\s*=\s*([\d\.]+) secs"

	input_threads = []
	input_sizes = []
	throughputs = []
	resp_times = []

	for line in lines:
		input_found = re.findall(input_pat, line)
		throughput_found = re.findall(throughput_pat, line)
		respt_found = re.findall(respt_pat, line)

		if input_found:
			input_threads.append(int(input_found[0][0]))
			input_sizes.append(int(input_found[0][2]))

		if throughput_found:
			throughputs.append(float(throughput_found[0]))

		if respt_found:
			resp_times.append(float(respt_found[0]))


	print 	"threads = %s\n" \
			"sizes = %s\n" \
			"throughputs = %s\n" \
			"response_times = %s\n" \
			%(input_threads, input_sizes, throughputs, resp_times)

	return input_threads, input_sizes, throughputs, resp_times


def expt1():
	"""
	Experiment 1: Chooses the result files and generates figures
	"""
	# filename = sys.argv[1]
	result_file = "./expt1.txt"
	input_threads, input_sizes, throughputs, resp_times \
		= parse_output(result_file) 

	throughputs_MiB = [tp/2**20 for tp in throughputs]

	fig1 = pl.figure()
	fig1.set_tight_layout(True)
	fig1.add_subplot(221)
	
	pl.semilogx(input_sizes, throughputs_MiB, 
				'bo-', ms=MARKER_SIZE, mew=0, mec='b')
	pl.xlabel("fixed file size (Bytes)")
	pl.ylabel("throughput (MiB/sec)")
	pl.text(2E3, 27, "(A)")

	fig1.add_subplot(222)
	pl.loglog(input_sizes, resp_times, 
			  'mo-', ms=MARKER_SIZE, mew=0, mec='m')
	pl.xlabel("fixed file size (Bytes)")
	pl.ylabel("response time (sec)")
	pl.text(2E3, 500, "(B)")

	fig1.add_subplot(223)
	pl.semilogx(resp_times, throughputs_MiB, 
				'go-', ms=MARKER_SIZE, mew=0, mec='g')
	pl.xlabel("response time(sec)")
	pl.ylabel("throughput (MiB/sec)")
	pl.text(0.2, 27, "(C)")

	pl.tight_layout()
	pl.savefig("./figures/%s" % result_file.replace(".txt", ".pdf"))
	# pl.show()

def expt2():
	"""
	Experiment 3: Chooses the result files and generates figures
	"""
	result_file = "./expt2.txt"
	input_threads, input_sizes, throughputs, resp_times \
		= parse_output(result_file) 

	throughputs_MiB = [tp/2**20 for tp in throughputs]

	fig1 = pl.figure()
	fig1.set_tight_layout(True)
	fig1.add_subplot(221)	
	pl.plot(input_threads, throughputs_MiB, 
			'bo-', ms=MARKER_SIZE, mew=0, mec='b')
	pl.xlabel("threads")
	pl.ylabel("throughput (MiB/sec)")
	pl.xlim(0,40)
	pl.text(1, 93, "(A)")


	fig1.add_subplot(222)
	pl.plot(input_threads, resp_times, 
			'mo-', ms=MARKER_SIZE, mew=0, mec='m')
	pl.xlabel("threads")
	pl.ylabel("response time (sec)")
	pl.xlim(0,40)
	pl.ylim(0.5,6)
	pl.text(0.5, 5.6, "(B)")

	fig1.add_subplot(223)
	pl.plot(resp_times, throughputs_MiB, 
			'go-', ms=MARKER_SIZE, mew=0, mec='g')
	pl.xlabel("response time (sec)")
	pl.ylabel("throughput (MiB/sec)")
	pl.xlim(0.5,6)
	pl.text(0.5, 93, "(C)")

	pl.tight_layout()
	pl.savefig("./figures/%s" % result_file.replace(".txt", ".pdf"))
	# pl.show()	

def expt3():
	"""
	Experiment 2: Chooses the result files and generates figures
	"""
	result_file = "./expt3.txt"
	input_threads, input_sizes, throughputs, resp_times \
		= parse_output(result_file) 

	throughputs_MiB = [tp/2**20 for tp in throughputs]

	fig1, (ax0, ax1) = pl.subplots(ncols=2, figsize=(6, 3))
	fig1.set_tight_layout(True)

	ax0.plot(input_threads, throughputs_MiB, 
			'bo-', ms=MARKER_SIZE, mew=0, mec='b')
	ax0.set_xlabel("threads")
	ax0.set_ylabel("throughput (MiB/sec)")
	ax0.set_xlim(0,25)
	ax0.text(1, 85, "(A)")

	ax1.plot(input_threads, resp_times, 
			'mo-', ms=MARKER_SIZE, mew=0, mec='m')
	ax1.set_xlabel("threads")
	ax1.set_ylabel("response time (sec)")
	ax1.set_xlim(0,25)
	ax1.set_ylim(0,400)
	ax1.text(1, 375, "(B)")

	pl.tight_layout()
	pl.savefig("./figures/%s" % result_file.replace(".txt", ".pdf"))
	# pl.show()	

def expt4():
	"""
	Experiment 4: Chooses the result files and generates figures
	"""	
	result_file = "./expt4.txt"
	input_threads_ff, input_sizes_ff, throughputs_ff, resp_times_ff \
		= parse_output(result_file) 

	input_threads_fs, input_sizes_fs, throughputs_fs, resp_times_fs \
	= parse_output("./expt3.txt") 

	throughputs_ff_MiB = [tp/2**20 for tp in throughputs_ff]
	throughputs_fs_MiB = [tp/2**20 for tp in throughputs_fs]

	fig1, (ax0, ax1) = pl.subplots(ncols=2, figsize=(6, 3))
	fig1.set_tight_layout(True)

	ax0.plot(input_threads_ff, throughputs_ff_MiB, 'bo-', ms=MARKER_SIZE, mew=0, mec='b')
	ax0.plot(input_threads_fs, throughputs_fs_MiB, 'bo--', ms=MARKER_SIZE, mew=MARKER_EDGE_WIDTH, mec='b', mfc="w")
	
	ax0.set_xlabel("threads")
	ax0.set_ylabel("throughput (MiB/sec)")
	ax0.set_xlim(0,25)
	ax0.set_ylim(0,130)
	ax0.legend(["FIXED_FILE", "FIXED_SIZE"], fontsize=8, frameon=False, loc=4)
	ax0.text(1, 120, "(A)")


	# fig1.add_subplot(122)
	ax1.plot(input_threads_ff, resp_times_ff, 'mo-', ms=MARKER_SIZE, mew=0, mec="m")
	ax1.plot(input_threads_fs, resp_times_fs, 'mo--', ms=MARKER_SIZE, mew=MARKER_EDGE_WIDTH, mec="m", mfc="w")	
	ax1.set_xlabel("threads")
	ax1.set_ylabel("response time (sec)")
	ax1.set_xlim(0,25)
	ax1.set_ylim(0,400)
	ax1.legend(["FIXED_FILE", "FIXED_SIZE"], fontsize=8, frameon=False)
	ax1.text(1, 375, "(B)")

	pl.tight_layout()
	pl.savefig("./figures/%s" % result_file.replace(".txt", ".pdf"))
	# pl.show()	
if __name__ == '__main__':
	expt1()
	expt2()
	expt3()
	expt4()

