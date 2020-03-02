#!/usr/bin/python3

from subprocess import Popen, PIPE
import subprocess
import os

domain = 'interval' 	# options are interval, octagon
# num_tests = 29
benchmarks = ['CO-2+2W_2', 'CO-2+2W_3', 'CO-2+2W_5', 'fibonacci', 'szymanski', 'dijkstra']
# benchmarks = ['CO-2+2W_2', 'CO-2+2W_3', 'CO-2+2W_5', 'fibonacci']
print('Name, Time, Iterations')

for benchmark in benchmarks:
	command = ['opt', '-load', 'build/interp/VerifierPass.so', '-verifier', '-'+domain, 
				'-no-print', '-useMOPO', 'tests/benchmarks/' + benchmark + '.ll']
	process = Popen(command, stdout=PIPE, stderr=PIPE)
	out, err = process.communicate()
	if ('ERROR:' in str(err) or 'core dumped' in str(err)):
		print('\033[91m'+benchmark,', Something went Wrong\033[0m')
	else:
		start_index_of_time = str(err).find('Time elapsed: ') + len('Time elapsed: ')
		end_index_of_time = start_index_of_time + str(err)[start_index_of_time: ].find(r'\n')
		print(benchmark, ',', str(err)[start_index_of_time: end_index_of_time])
		# start_index_of_iter = end_index_of_time + str(err)[end_index_of_time: ].find('#iterations: ') + len('#iterations: ')
		# end_index_of_iter = start_index_of_iter + str(err)[start_index_of_iter: ].find(r'\n')
		# print(benchmark, ',', str(err)[start_index_of_time : end_index_of_time], ',', str(err)[start_index_of_iter : end_index_of_iter])
		
duration = .1  # seconds
freq = 440  # Hz
os.system('play -nq -t alsa synth {} sine {}'.format(duration, freq))


