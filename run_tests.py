#!/usr/bin/python3

from subprocess import Popen, PIPE
import subprocess
import os

domain = 'octagon' 	# options are interval, octagon
num_tests = 31
interfcomb = False
# if the test should fail assertion, value of test_result is false]
test_result = 	[False, True, 	False, 	True, 	True, 
				True, 	False, 	True, 	True, 	True, 
				True,	True, 	True,	True,	True,
				True, 	False, 	True, 	True, 	False,
				True, 	False,	True, 	True,	True,
				True, 	True, 	True,	True,	True,
				True]
num_correct = 0
num_false_positive = 0
num_missed_asserts = 0

# testsRA = [16, 30, 31, 32, 33, 34, 35]

for test_id in range(1, num_tests+1):
	command = ['opt', '-load', 'build/interp/VerifierPass.so', '-verifier', '-'+domain, 
				'-no-print', '-stop-on-fail', '-eager-pruning', 'tests/litmus/test' + str(test_id) + '.ll']
	if not interfcomb:
		command.append('-no-interf-comb')
	process = Popen(command, stdout=PIPE, stderr=PIPE)
	out, err = process.communicate()
	# print(err)
	if ('ERROR:' in str(err)):
		print('\033[91mTest'+str(test_id),': Something went Wrong\033[0m')
	elif ('Assertion failed' in str(err)) == (not test_result[test_id-1]):
		print('Test'+str(test_id),": Pass")
		num_correct = num_correct+1
	else:
		if test_result[test_id-1]:
			print('Test'+str(test_id),": Fail (false positive)")
			num_false_positive = num_false_positive+1
		else:
			print('Test'+str(test_id),": Fail (Assertion missed)")
			num_missed_asserts = num_missed_asserts+1

print('_'*50)
print('Tests passed correctly:', num_correct)
print('Tests with false positive:', num_false_positive)
if (num_missed_asserts != 0):
	print('\033[91mTests with MISSED ASSERT:', num_missed_asserts, '\033[0m')
else:
	print('Tests with MISSED ASSERT:', num_missed_asserts)

duration = .05  # seconds
freq = 440  # Hz
os.system('play -nq -t alsa synth {} sine {}'.format(duration, freq))
