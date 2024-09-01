echo hello, testing redirect functions
cd testCases
wc -l < linecounter.txt
echo should output 6, for 6 lines in txt file
echo hi guys > output.txt
echo check output.txt file in testCases to see if "hi guys" is correctly outputted there
wc -l < linecounter.txt > output.txt
echo check output.txt again to see if only 6 is there from wc -l
exit
