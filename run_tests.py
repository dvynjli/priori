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

for test_id in range(1, num_tests+1):
	command = ['opt', '-load', 'build/interp/VerifierPass.so', '-verifier', '-'+domain, '-z3-minimal', '-no-print', 'tests/test' + str(test_id) + '.ll']
	process = Popen(command, stdout=PIPE, stderr=PIPE)
	out, err = process.communicate()
	if 'ERROR' in str(err):
		if test_result[test_id-1] == False:
			print('Test'+str(test_id),": Pass")
		else:
			print('Test'+str(test_id),": Fail")
	else:
		if test_result[test_id-1] == True:
			print('Test'+str(test_id),": Pass")
		else:
			print('Test'+str(test_id),": Fail")
