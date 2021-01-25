#!/usr/bin/python3

from subprocess import Popen, PIPE
import subprocess
import os
import sys
import datetime

num_runs = 4        # reported time is avg of these many runs
unwind = 1          # loop unrolling in cbmc
view_switch = 2     # number of viwe-switched to be added

benchmarks = ['dijkstra', 'bakery', 'burns', 'dekker', 'dekker_sim', 
				'lamport', 'peterson', 'peterson3']

# benchmarks = ['dijkstra']
# benchmarks = ['peterson3']

def is_bug_found(out):
	# print(out.find('VERIFICATION FAILED'))
	if out.find('VERIFICATION FAILED') != -1:
		return True
	elif out.find('VERIFICATION SUCCESSFUL'):
		return False
	else:
		print('something went wrong during cbmc execution. please check out')

def find_time_buggy(err):
	# end_err = err[err.find('VERIFICATION FAILED')+len('VERIFICATION FAILED'): ]
	# print('\nend_err:',end_err)
	str_time = err[err.find('system 0:')+len('system 0:'): ]
	# print('\nstr_time:',str_time)
	time = str_time[: str_time.find('elapsed')]
	# print('time:', time)
	return float(time)

def find_time_non_buggy(out):
	start_index_of_time = str(out).find('Runtime decision procedure: ') + len('Runtime decision procedure: ')
	end_index_of_time = start_index_of_time + str(out)[start_index_of_time: ].find('s')
	time = float(str(out)[start_index_of_time: end_index_of_time])
	# print(time)
	return time

def run_command(command):
    process = Popen(command, stdout=PIPE, stderr=PIPE)
    # print('command ran')
    out, err = process.communicate()
    return (str(out), str(err))

def compile(benchmark, vb):
	command = ['./cseq-feeder.py', '-i', 'abs_interp_tests/benchmarks/'+benchmark+'.c', '--viewSwitches', str(vb)]
	# print('running command: ', command)
	out, err = run_command(command)
	# print('out: ', out)
	# print('err: ', err)
	if out.find('compilation done') != -1:
		# print("successfully compiled")
		pass
	else:
		print("something went wrong during cseq compilation\n")
		exit(0)

def run_cbmc(benchmark):
	command = ['time', 'cbmc', 'abs_interp_tests/benchmarks/_cs_'+benchmark+'.c', '--unwind', str(unwind), '--stop-on-fail']
	# print('running command: ', command)
	out,err = run_command(command)
	# print('out: ', out)
	# print('err: ', err)
	return out,err

def run_till_bug_found(benchmark):
	vb = 2
	compile(benchmark, vb)
	out,err = run_cbmc(benchmark)
	# print(is_bug_found(out))
	# time = find_time_non_buggy(out)
	time = 0
	while(not is_bug_found(out)):
		time = find_time_non_buggy(out)
		vb = vb+1
		compile(benchmark, vb)
		out,err = run_cbmc(benchmark)
	time = time + find_time_buggy(err)
	return (time, vb)


print('Results generated at ', str(datetime.datetime.now()))
print('Name & time & vb')
# run all benchmarks
for benchmark in benchmarks:
	time, vb = run_till_bug_found(benchmark)
	print('%s & %.2f & %d'  % (benchmark, time, vb))
	# runs = 1
    # for i in range(num_runs-1):
    #     process = Popen(command, stdout=PIPE, stderr=PIPE)
    #     out, err = process.communicate()
    #     start_index_of_time = str(out).find('Runtime decision procedure: ') + len('Runtime decision procedure: ')
    #     end_index_of_time = start_index_of_time + str(out)[start_index_of_time: ].find('s')
    #     time = time + float(str(out)[start_index_of_time: end_index_of_time])
    #     runs = runs + 1
    # # print(benchmark, ',', time/runs)
    
    # start_index_of_bug = end_index_of_time + str(out)[end_index_of_time: ].find('VERIFICATION ')
    # end_index_of_bug = start_index_of_bug + str(out)[start_index_of_bug: ].find(r'\n')
    # bug = 'No'
    # # print (str(out)[start_index_of_bug:end_index_of_bug])
    # if (str(out)[start_index_of_bug:end_index_of_bug]) == 'VERIFICATION FAILED':
    #     bug = 'Yes'
    # print(benchmark, '&', time/runs, '&', bug, '\\\\')
