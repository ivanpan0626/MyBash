echo hello, testing pipe functions
cd testCases
rm t1t.txt | rm t4t.txt
echo removed all files
touch t1t.txt | touch t4t.txt
echo readded all files
ls | wc
cd ..
ls | grep my
exit
