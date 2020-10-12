#!/usr/bin/python3

from subprocess import Popen, PIPE
import subprocess
import os
import sys
import datetime

domain = 'interval'		 # options are interval, octagon
num_runs = 4 
interfcomb = False
precision = 'P3' 

buggy = ['dijkstra', 'bakery', 'burns', 'dekker', 'dekker_sim', 
			'lamport', 'peterson', 'peterson3', 
                        '10R1W', '15R1W', 'szymanski_7_buggy']
non_buggy = ['CO-2+2W_5', 'CO-2+2W_10', 'CO-2+2W_15', 'fibonacci', 
			'dijkstra_fen', 'bakery_fen', 'burns_fen',  
			'lamport_fen','peterson_fen', 'tbar_fen', # 'dekker_fen', 'tbar_fenced_2'] 
			'peterson3_fen','redundant_co' , 'gcd', 'pthread_demo', 'exponential_bug_6',
			'exponential_bug_7', 'exponential_bug_8','exponential_bug_9']
#if len(sys.argv) == 1 or sys.argv[1] == '-all' or sys.argv[1] == '-a':
#		benchmarks = buggy + non_buggy
#elif sys.argv[1] == '-buggy':
#		benchmarks = buggy
#elif sys.argv[1] == '-non-buggy':
#		benchmarks = non_buggy
#else:
#		print('Flags: ""/-vuf/-vf/-tnb/-a')
#		exit(0)


def bug_found(err):
	return 'Assertion failed' in err

def find_time(benchmark, out, err):
	if ('ERROR:' in err or 'core dumped' in err):
		print('\033[91m'+benchmark,', Something went Wrong\033[0m')
	else:
		start_index_of_time = err.find('Time elapsed: ') + len('Time elapsed: ')
		end_index_of_time = start_index_of_time + err[start_index_of_time: ].find(r'\n')
		time = float(err[start_index_of_time: end_index_of_time])
		return time

def find_num_iter(err):
	start_index_of_iter = err.rfind('#iterations: ') + len('#iterations: ')
	end_index_of_iter = start_index_of_iter + err[start_index_of_iter: ].find(r'\n')
	return err[start_index_of_iter : end_index_of_iter]


def run_benchmarks(benchmark_list, isBuggy):
	for benchmark in benchmark_list:
		command = ['/usr/bin/opt', '-load', 'build/interp/VerifierPass.so', '-verifier', '-'+domain, 
					    '-no-print', '-stop-on-fail', '-eager-pruning', '-'+precision,
                                            'tests/benchmarks/' + benchmark + '.ll']
		if (not interfcomb):
				command.append('-no-interf-comb')
		time = 0
		for i in range(num_runs):
			process = Popen(command, stdout=PIPE, stderr=PIPE)
			out, err = process.communicate()
			cur_buggy = bug_found(str(err))
			if i==0:
				if cur_buggy and not isBuggy:
					print(benchmark, '&', 'False positive' , '\\\\')
					break
				elif not cur_buggy and isBuggy:
					print(benchmark, '&', 'Assertion missed' , '\\\\')
					break
			time = time + find_time(benchmark, str(out), str(err))
		iterations = find_num_iter(str(err))
		print(benchmark, '&', time/num_runs, '&', iterations, '\\\\')


print('Results generated at ', str(datetime.datetime.now()))
print('Precision:', precision, '\tDomain:',domain)
print('Name & Time & Iterations & Time & Iterations \\\\')
if len(sys.argv) == 1 or sys.argv[1] == '-all':
	run_benchmarks(buggy, True)
	run_benchmarks(non_buggy, False)
elif sys.argv[1] == '-buggy':
	run_benchmarks(buggy, True)
elif sys.argv[1] == '-non-buggy':
	run_benchmarks(non_buggy, False)
else:
	print('Flags: "/-buggy/-non-buggy/-all or nothing"')
	exit(0)


# duration = .1  # seconds
# freq = 440  # Hz
# os.system('play -nq -t alsa synth {} sine {}'.format(duration, freq))

