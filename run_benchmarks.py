#!/usr/bin/python3

from subprocess import Popen, PIPE
import subprocess
import os
import sys
import datetime

domain = 'interval' 	# options are interval, octagon
num_runs = 4

# following benchamrks are unfenced version of mutex algorithms from VBMC and have errors
vbmc_unfenced = ['bakery_assume_unfenced', 'burns_assume_unfenced', 'dekker_assume_unfenced', 
						'lamport_assume_unfenced', 'peterson_assume_unfenced']
vbmc_fenced = ['bakery_assume_fenced', 'burns_assume_fenced', 'peterson_assume_fenced', 
				'tbar_fenced', 'tbar_fenced_2', 'lamport_assume_fenced']
# following benchmarks do not have errors and are from Tracer
tracer_no_bug = ['CO-2+2W_2', 'CO-2+2W_3', 'CO-2+2W_5', 'fibonacci', 'dijkstra']
benchmarks = []

if len(sys.argv) == 1 or sys.argv[1] == '-all' or sys.argv[1] == '-a':
	benchmarks = vbmc_unfenced + vbmc_fenced + tracer_no_bug
elif sys.argv[1] == '-vbmc-febced' or sys.argv[1] == '-vf':
	benchmarks = vbmc_fenced
elif sys.argv[1] == '-tracer-no-bug' or sys.argv[1] == '-tnb':
	benchmarks = tracer_no_bug
elif sys.argv[1] == '-vbmc-unfenced' or sys.argv[1] == '-vuf':
	benchmarks = vbmc_unfenced
else:
	print('Flags: ""/-vuf/-vf/-tnb/-a')
	exit(0)

print('Results generated at ', str(datetime.datetime.now()))
print('Name, Time, Iterations')

for benchmark in benchmarks:
	command = ['opt', '-load', 'build/interp/VerifierPass.so', '-verifier', '-'+domain, 
				'-no-print', '-useMOPO', '-stop-on-fail', '-eager', 'tests/benchmarks/' + benchmark + '.ll']
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
		print(benchmark, ',', time/runs, ',', str(err)[start_index_of_iter : end_index_of_iter])
		
duration = .1  # seconds
freq = 440  # Hz
os.system('play -nq -t alsa synth {} sine {}'.format(duration, freq))


