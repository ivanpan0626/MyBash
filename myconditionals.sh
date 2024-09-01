cd nonExistentDirectory
else pwd
then echo this line and pwd should run if cd fails
cd testCases
then pwd
then echo directory should be successfully changed
ls nonexistentfile
else pwd
echo testing for double else
ls firstfailed
else ls secondfailed
else pwd
exit
