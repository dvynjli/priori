#!/usr/bin/python3

from subprocess import Popen, PIPE
import subprocess
import os
import sys
import datetime

num_runs = 4        # reported time is avg of these many runs
unwind = 1          # loop unrolling in cbmc
view_switch = 2     # number of viwe-switched to be added

# following benchamrks are unfenced version of mutex algorithms from VBMC and have errors
# szymanski and peterson_3 are left
vbmc_unfenced = ['bakery_assume_unfenced', 'burns_assume_unfenced', 'dekker_assume_unfenced', 
						'lamport_assume_unfenced', 'peterson_assume_unfenced', 
						'peterson_assume_unfenced_thr3', 'dekker_simple_unfenced'] 
# following benchamrks are version of mutex algorithms from VBMC and do not have errors
# 
vbmc_fenced = ['bakery_assume_fenced', 'peterson_assume_fenced', 
				'tbar_fenced', 'tbar_fenced_2', 'lamport_assume_fenced']
# following benchmarks do not have errors and are from Tracer
tracer_no_bug = ['CO-2+2W_2', 'CO-2+2W_3', 'CO-2+2W_5', 'fibonacci', 'dijkstra',
				'burns_assume_fenced']
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

# print('Results generated at ', str(datetime.datetime.now()))
print('Name & Time & Bug Found \\\\')

benchmarks=vbmc_unfenced
# compile all benchmarks
for benchmark in benchmarks:
    command = ['./cseq-feeder.py', '-i', 'abs_interp_tests/benchmarks/'+benchmark+'.c', '--viewSwitches', str(view_switch)]
    process = Popen(command, stdout=PIPE, stderr=PIPE)
    out, err = process.communicate()
    # print('out:', out)
    # print('err:', err)

# run all benchmarks
for benchmark in benchmarks:
    command = ['cbmc', 'abs_interp_tests/benchmarks/_cs_'+benchmark+'.c', '--unwind', str(unwind), '--stop-on-fail']
    process = Popen(command, stdout=PIPE, stderr=PIPE)
    out, err = process.communicate()
    # print('out:', out.decode('unicode_escape'))
    # print('err:', err.decode('unicode_escape'))
    start_index_of_time = str(out).find('Runtime decision procedure: ') + len('Runtime decision procedure: ')
    end_index_of_time = start_index_of_time + str(out)[start_index_of_time: ].find('s')
    time = float(str(out)[start_index_of_time: end_index_of_time])
    runs = 1
    for i in range(num_runs-1):
        process = Popen(command, stdout=PIPE, stderr=PIPE)
        out, err = process.communicate()
        start_index_of_time = str(out).find('Runtime decision procedure: ') + len('Runtime decision procedure: ')
        end_index_of_time = start_index_of_time + str(out)[start_index_of_time: ].find('s')
        time = time + float(str(out)[start_index_of_time: end_index_of_time])
        runs = runs + 1
    # print(benchmark, ',', time/runs)
    
    start_index_of_bug = end_index_of_time + str(out)[end_index_of_time: ].find('VERIFICATION ')
    end_index_of_bug = start_index_of_bug + str(out)[start_index_of_bug: ].find(r'\n')
    bug = 'No'
    # print (str(out)[start_index_of_bug:end_index_of_bug])
    if (str(out)[start_index_of_bug:end_index_of_bug]) == 'VERIFICATION FAILED':
        bug = 'Yes'
    print(benchmark, '&', time/runs, '&', bug, '\\\\')
