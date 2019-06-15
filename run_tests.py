#!/usr/bin/python3

from subprocess import Popen, PIPE
import subprocess
import os

domain = 'interval'
num_tests=28
# if the test should fail assertion, value of test_result is false]
test_result = 	[True, True, True, True, True, 
				True, False, True, True, False, 
				True, False, False, False, True, 
				False, True, True, True, False, 
				False, True, True, True, True, 
				True, True, True]
num_correct = 0
num_false_positive = 0
num_missed_asserts = 0

for test_id in range(1, num_tests+1):
	command = ['opt', '-load', 'build/interp/VerifierPass.so', '-verifier', '-'+domain, '-z3-minimal', '-no-print', 'tests/test' + str(test_id) + '.ll']
	process = Popen(command, stdout=PIPE, stderr=PIPE)
	out, err = process.communicate()
	if ('ERROR' in str(err)) == (not test_result[test_id-1]):
		print('Test'+str(test_id),": Pass")
		num_correct = num_correct+1
	else:
		if test_result[test_id-1]:
			print('Test'+str(test_id),": Fail false positive")
			num_false_positive = num_false_positive+1
		else:
			print('Test'+str(test_id),": Fail Assertion missed")
			num_missed_asserts = num_missed_asserts+1

print('_'*50)
print('Tests passed correctly:', num_correct)
print('Tests with false positive:', num_false_positive)
print('Tests with MISSED ASSERT:', num_missed_asserts)
