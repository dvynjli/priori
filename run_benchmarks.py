#!/usr/bin/python3

from subprocess import Popen, PIPE
import subprocess
import os
import sys
import datetime

domain = 'octagon' 	# options are interval, octagon
num_runs = 4
interfcomb = False

buggy = ['dijkstra', 'bakery', 'burns', 'dekker', 'dekker_sim', 
				'lamport', 'peterson', 'peterson3']
non_buggy = ['CO-2+2W_2', 'CO-2+2W_3', 'CO-2+2W_5', 'fibonacci', 
				'dijkstra_fen', 'bakery_fen', 'burns_fen',  
				'lamport_fen','peterson_fen', 'tbar_fen' # 'dekker_fen', 'tbar_fenced_2'] 
				'gcd', 'pthread_demo', 'exponential_bug_6',
				'exponential_bug_7', 'exponential_bug_8','exponential_bug_9']
benchmarks = []

if len(sys.argv) == 1 or sys.argv[1] == '-all' or sys.argv[1] == '-a':
	benchmarks = buggy + non_buggy
elif sys.argv[1] == '-buggy':
	benchmarks = buggy
elif sys.argv[1] == '-non-buggy':
	benchmarks = non_buggy
else:
	print('Flags: ""/-vuf/-vf/-tnb/-a')
	exit(0)

print('Results generated at ', str(datetime.datetime.now()))
print('Name & Time & Iterations & Time & Iterations \\\\')

for benchmark in benchmarks:
	command = ['/usr/bin/opt', '-load', 'build/interp/VerifierPass.so', '-verifier', '-'+domain, 
				'-no-print', '-stop-on-fail', '-eager-pruning', 'tests/benchmarks/' + benchmark + '.ll']
	if (not interfcomb):
		command.append('-no-interf-comb')
	process = Popen(command, stdout=PIPE, stderr=PIPE)
	out, err = process.communicate()
	if ('ERROR:' in str(err) or 'core dumped' in str(err)):
		print('\033[91m'+benchmark,', Something went Wrong\033[0m')
	else:
		start_index_of_time = str(err).find('Time elapsed: ') + len('Time elapsed: ')
		end_index_of_time = start_index_of_time + str(err)[start_index_of_time: ].find(r'\n')
		time = float(str(err)[start_index_of_time: end_index_of_time])
		runs = 1
		for i in range(num_runs-1):
			process = Popen(command, stdout=PIPE, stderr=PIPE)
			out, err = process.communicate()
			start_index_of_time = str(err).find('Time elapsed: ') + len('Time elapsed: ')
			end_index_of_time = start_index_of_time + str(err)[start_index_of_time: ].find(r'\n')
			time = time + float(str(err)[start_index_of_time: end_index_of_time])
			runs = runs + 1
		# print(benchmark, ',', time/runs)
		
		start_index_of_iter = end_index_of_time + str(err)[end_index_of_time: ].find('#iterations: ') + len('#iterations: ')
		end_index_of_iter = start_index_of_iter + str(err)[start_index_of_iter: ].find(r'\n')
		print(benchmark, '&', time/runs, '&', str(err)[start_index_of_iter : end_index_of_iter], '\\\\')
		
# duration = .1  # seconds
# freq = 440  # Hz
# os.system('play -nq -t alsa synth {} sine {}'.format(duration, freq))


