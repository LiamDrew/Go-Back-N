#!/bin/tcsh

# output colors
set GREEN = "\033[0;32m"
set RED = "\033[0;31m"
set BLUE = "\033[0;34m"
set NC = "\033[0m"

# counter for passed tests
set passed_tests = 0
set total_tests = 19
set IP_ADDRESS = 10.4.2.20
set PORT = 9052

echo "${BLUE}Starting Tests...${NC}"

# Test 1: Request existing text file with a window size of 9
echo "${BLUE}=====================================================================${NC}"
echo "${BLUE}=== Running Test 1: Request existing text file with window size 9 ===${NC}"
./normalClient ${IP_ADDRESS} ${PORT} test.txt 9 >& /dev/null
test -f test.txt
if ( $? == 0 ) then
    grep -q "TOTO RINA" test.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 1 Passed: File 'test.txt' exists and contains expected content 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 1 Failed: File 'test.txt' does not contain expected content 😓${NC}"
    endif
else
    echo "\n${RED}Test 1 Failed: File 'test.txt' does not exist 😓${NC}"
endif

rm -f test.txt

echo "${BLUE}=====================================================================${NC}"

# Test 2: Request existing text file with a window size of 1
echo "${BLUE}=== Running Test 2: Request existing text file with window size 1 ===${NC}"
./normalClient ${IP_ADDRESS} ${PORT} test.txt 1 >& /dev/null
test -f test.txt
if ( $? == 0 ) then
    grep -q "TOTO RINA" test.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 2 Passed: File 'test.txt' exists and contains expected content 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 2 Failed: File 'test.txt' does not contain expected content 😓${NC}"
    endif
else
    echo "\n${RED}Test 2 Failed: File 'test.txt' does not exist 😓${NC}"
endif

rm -f test.txt
echo "${BLUE}=====================================================================${NC}"

# Test 3: Request non-existent file
echo "${BLUE}=== Running Test 3: Request non-existent file ===${NC}"
set temp_output_file = "/tmp/normalClient_output.txt"
./normalClient ${IP_ADDRESS} ${PORT} unexistantFile.txt 1 >& $temp_output_file

# cat $temp_output_file

grep -q "Received ERROR packet from server." $temp_output_file
if ( $? == 0 ) then
    echo "\n${GREEN}Test 3 Passed: Received expected error message 🎉${NC}"
    @ passed_tests++
else
    echo "\n${RED}Test 3 Failed: Expected error message not found 😓${NC}"
endif

rm -f $temp_output_file

echo "${BLUE}=====================================================================${NC}"

# Test 4: Request existing file with window size 1, simulating loss of the first packet
echo "${BLUE}=== Running Test 4: Request existing text file with window size 1, simulating loss of first packet ===${NC}"
./lossSimulation ${IP_ADDRESS} ${PORT} test1.txt 1 0 >& /dev/null
test -f test1.txt
if ( $? == 0 ) then
    grep -q "Ciro Di Marzio" test1.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 4 Passed: File 'test1.txt' exists and contains expected content 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 4 Failed: File 'test1.txt' does not contain expected content 😓${NC}"
    endif
else
    echo "\n${RED}Test 4 Failed: File 'test1.txt' does not exist 😓${NC}"
endif
rm -f test1.txt

# Test 5: Request existing file with window size 9, simulating loss of first packet
echo "${BLUE}=====================================================================${NC}"

echo "${BLUE}=== Running Test 5: Request existing text file with window size 9, simulating loss of first packet ===${NC}"
./lossSimulation ${IP_ADDRESS} ${PORT} test.txt 9 0 >& /dev/null
test -f test.txt
if ( $? == 0 ) then
    grep -q "TOTO RINA" test.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 5 Passed: File 'test.txt' exists and contains expected content 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 5 Failed: File 'test.txt' does not contain expected content 😓${NC}"
    endif
else
    echo "\n${RED}Test 5 Failed: File 'test.txt' does not exist 😓${NC}"
endif

rm -f test.txt

echo "${BLUE}=====================================================================${NC}"

# Test 6: Request existing binary file with window size 1
echo "${BLUE}=== Running Test 6: Request existing binary file with window size 1 ===${NC}"
./normalClient ${IP_ADDRESS} ${PORT} play.jpg 1 >& /dev/null

# Check if play.jpg exists and matches existingPlay.jpg
test -f play.jpg
if ( $? == 0 ) then
    cmp -s play.jpg existingPlay.jpg
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 6 Passed: File 'play.jpg' exists and matches 'existingPlay.jpg' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 6 Failed: File 'play.jpg' does not match 'existingPlay.jpg' 😓${NC}"
    endif
else
    echo "\n${RED}Test 6 Failed: File 'play.jpg' was not created 😓${NC}"
endif

rm -f play.jpg
echo "${BLUE}=====================================================================${NC}"

# Test 7: Request existing binary file with window size 1
echo "${BLUE}=== Running Test 7: Request existing binary file with window size 9 ===${NC}"
./normalClient ${IP_ADDRESS} ${PORT} play.jpg 9 >& /dev/null

# Check if play.jpg exists and matches existingPlay.jpg
test -f play.jpg
if ( $? == 0 ) then
    cmp -s play.jpg existingPlay.jpg
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 7 Passed: File 'play.jpg' exists and matches 'existingPlay.jpg' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 7 Failed: File 'play.jpg' does not match 'existingPlay.jpg' 😓${NC}"
    endif
else
    echo "\n${RED}Test 7 Failed: File 'play.jpg' was not created 😓${NC}"
endif

rm -f play.jpg
echo "${BLUE}=====================================================================${NC}"

# Test 8: Request existing binary file with window size 5
echo "${BLUE}=== Running Test 8: Request existing binary file with window size 5 ===${NC}"
./normalClient ${IP_ADDRESS} ${PORT} Eduardito.jpg 5 >& /dev/null

# Check if Eduardito.jpg exists and matches existingEduardito.jpg
test -f Eduardito.jpg
if ( $? == 0 ) then
    cmp -s Eduardito.jpg existingEduardito.jpg
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 8 Passed: File 'Eduardito.jpg' exists and matches 'existingEduardito.jpg' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 8 Failed: File 'Eduardito.jpg' does not match 'existingEduardito.jpg' 😓${NC}"
    endif
else
    echo "\n${RED}Test 8 Failed: File 'Eduardito.jpg' was not created 😓${NC}"
endif

rm -f Eduardito.jpg
echo "${BLUE}=====================================================================${NC}"

# Test 9:  Request existing binary file with window size 1, simulating loss of first packet
echo "${BLUE}=== Running Test 9: Request existing binary file with window size 1, simulating loss of first packet ===${NC}"
./lossSimulation ${IP_ADDRESS} ${PORT} Eduardito.jpg 1 0 >& /dev/null

# Check if Eduardito.jpg exists and matches existingEduardito.jpg
test -f Eduardito.jpg
if ( $? == 0 ) then
    cmp -s Eduardito.jpg existingEduardito.jpg
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 8 Passed: File 'Eduardito.jpg' exists and matches 'existingEduardito.jpg' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 8 Failed: File 'Eduardito.jpg' does not match 'existingEduardito.jpg' 😓${NC}"
    endif
else
    echo "\n${RED}Test 8 Failed: File 'Eduardito.jpg' was not created 😓${NC}"
endif

rm -f Eduardito.jpg
echo "${BLUE}=====================================================================${NC}"

# Test 10:  Request existing binary file with window size 1, simulating loss of EOF packet
echo "${BLUE}=== Running Test 10: Request existing binary file with window size 1, simulating loss of EOF packet ===${NC}"
./lossSimulation ${IP_ADDRESS} ${PORT} Eduardito.jpg 1 42 >& /dev/null

# Check if Eduardito.jpg exists and matches existingEduardito.jpg
test -f Eduardito.jpg
if ( $? == 0 ) then
    cmp -s Eduardito.jpg existingEduardito.jpg
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 10 Passed: File 'Eduardito.jpg' exists and matches 'existingEduardito.jpg' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 10 Failed: File 'Eduardito.jpg' does not match 'existingEduardito.jpg' 😓${NC}"
    endif
else
    echo "\n${RED}Test 10 Failed: File 'Eduardito.jpg' was not created 😓${NC}"
endif

rm -f Eduardito.jpg
echo "${BLUE}=====================================================================${NC}"

# Test 11: Request existing binary file with window size 1, simulating loss of 5 random packets
echo "${BLUE}=== Running Test 11: Request existing binary file with window size 1, simulating loss of 5 random packets ===${NC}"
echo "${BLUE}=== ⚠️ This test takes some time because of the loss simulation ⚠️ ===${NC}"

./lossSimulation ${IP_ADDRESS} ${PORT} MrMIT.jpeg 1 2,7,8,10,20 >& /dev/null

# Check if MrMIT.jpeg exists and matches existingMrMIT.jpeg
test -f MrMIT.jpeg
if ( $? == 0 ) then
    cmp -s MrMIT.jpeg existingMrMIT.jpeg
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 11 Passed: File 'MrMIT.jpeg' exists and matches 'existingMrMIT.jpeg' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 11 Failed: File 'MrMIT.jpeg' does not match 'existingMrMIT.jpeg' 😓${NC}"
    endif
else
    echo "\n${RED}Test 11 Failed: File 'MrMIT.jpeg' was not created 😓${NC}"
endif

rm -f MrMIT.jpeg
echo "${BLUE}=====================================================================${NC}"

# Test 12: Request existing binary file with window size 7, simulating loss of 3 random packets
echo "${BLUE}=== Running Test 12: Request existing binary file with window size 7, simulating loss of 3 random packets ===${NC}"
echo "${BLUE}=== ⚠️ This test takes some time because of the loss simulation ⚠️ ===${NC}"

./lossSimulation ${IP_ADDRESS} ${PORT} MrMIT.jpeg 7 2,7,23 >& /dev/null

# Check if MrMIT.jpeg exists and matches existingMrMIT.jpeg
test -f MrMIT.jpeg
if ( $? == 0 ) then
    cmp -s MrMIT.jpeg existingMrMIT.jpeg
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 12 Passed: File 'MrMIT.jpeg' exists and matches 'existingMrMIT.jpeg' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 12 Failed: File 'MrMIT.jpeg' does not match 'existingMrMIT.jpeg' 😓${NC}"
    endif
else
    echo "\n${RED}Test 12 Failed: File 'MrMIT.jpeg' was not created 😓${NC}"
endif

rm -f MrMIT.jpeg
echo "${BLUE}=====================================================================${NC}"

# Test 13: Request existing binary file with window size 7, simulating loss of 2 random packets
echo "${BLUE}=== Running Test 13: Request existing binary file with window size 4, simulating loss of 2 random packets ===${NC}"
echo "${BLUE}=== ⚠️ This test takes some time because of the loss simulation ⚠️ ===${NC}"

./lossSimulation ${IP_ADDRESS} ${PORT} MrMIT.jpeg 7 9,15 >& /dev/null

# Check if MrMIT.jpeg exists and matches existingMrMIT.jpeg
test -f MrMIT.jpeg
if ( $? == 0 ) then
    cmp -s MrMIT.jpeg existingMrMIT.jpeg
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 13 Passed: File 'MrMIT.jpeg' exists and matches 'existingMrMIT.jpeg' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 13 Failed: File 'MrMIT.jpeg' does not match 'existingMrMIT.jpeg' 😓${NC}"
    endif
else
    echo "\n${RED}Test 13 Failed: File 'MrMIT.jpeg' was not created 😓${NC}"
endif

rm -f MrMIT.jpeg
echo "${BLUE}=====================================================================${NC}"

# Test 14: Request existing binary file with window size 7, no ACKs sent to trigger timeout
echo "${BLUE}=== Running Test 14: Request existing binary file with window size 7, no ACKs sent to trigger timeout ===${NC}"
echo "${BLUE}=== ⚠️ This test takes approx 19 seconds (3 seconds for each retry (5 retries) and 4 seconds wait to see if server sends again) because we test the server's timeout ⚠️ ===${NC}"
./timeoutTest ${IP_ADDRESS} ${PORT} MrMIT.jpeg 7 > timeoutTest.log

# Check if the server stopped after 5 retries based on log output
grep -q "Packet .* was received 5 times without an ACK" timeoutTest.log
if ( $? == 0 ) then
    grep -q "No further retransmissions from server. Server has stopped after 5 retries." timeoutTest.log
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 14 Passed: Server stopped after 5 retries when no ACKs were sent 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 14 Failed: Server did not stop after 5 retries 😓${NC}"
    endif
else
    echo "\n${RED}Test 14 Failed: Server did not reach 5 retries as expected 😓${NC}"
endif

# Clean up log file
rm -f timeoutTest.log

echo "${BLUE}=====================================================================${NC}"

# Test 15: Request existing binary file of size multiple to 512 without loss simulation
echo "${BLUE}=== Running Test 15: Request existing binary file of size multiple to 512 without loss simulation ===${NC}"
./normalClient ${IP_ADDRESS} ${PORT} 1024bytes.txt 2 >& /dev/null

# Check if 1024bytes.txt exists and matches existing1024bytes.txt
test -f 1024bytes.txt
if ( $? == 0 ) then
    cmp -s 1024bytes.txt existing1024bytes.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 15 Passed: File '1024bytes.txt' exists and matches 'existing1024bytes.txt' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 15 Failed: File '1024bytes.txt' does not match 'existing1024bytes.txt' 😓${NC}"
    endif
else
    echo "\n${RED}Test 15 Failed: File '1024bytes.txt' was not created 😓${NC}"
endif

# Clean up log file
rm -f 1024bytes.txt

echo "${BLUE}=====================================================================${NC}"

# Test 16: Request existing binary file of size multiple to 512 with loss simulation of first packet
echo "${BLUE}=== Running Test 16: Request existing binary file of size multiple to 512 with loss simulation of first packet ===${NC}"
./lossSimulation ${IP_ADDRESS} ${PORT} 1024bytes.txt 2 0 >& /dev/null

# Check if 1024bytes.txt exists and matches existing1024bytes.txt
test -f 1024bytes.txt
if ( $? == 0 ) then
    cmp -s 1024bytes.txt existing1024bytes.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 16 Passed: File '1024bytes.txt' exists and matches 'existing1024bytes.txt' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 16 Failed: File '1024bytes.txt' does not match 'existing1024bytes.txt' 😓${NC}"
    endif
else
    echo "\n${RED}Test 16 Failed: File '1024bytes.txt' was not created 😓${NC}"
endif

# Clean up log file
rm -f 1024bytes.txt

echo "${BLUE}=====================================================================${NC}"

# Test 17: Request existing binary file of size multiple to 512 with loss simulation of last packet
echo "${BLUE}=== Running Test 17: Request existing binary file of size multiple to 512 with loss simulation of last packet ===${NC}"
./lossSimulation ${IP_ADDRESS} ${PORT} 1024bytes.txt 2 2 >& /dev/null

# Check if 1024bytes.txt exists and matches existing1024bytes.txt
test -f 1024bytes.txt
if ( $? == 0 ) then
    cmp -s 1024bytes.txt existing1024bytes.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 17 Passed: File '1024bytes.txt' exists and matches 'existing1024bytes.txt' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 17 Failed: File '1024bytes.txt' does not match 'existing1024bytes.txt' 😓${NC}"
    endif
else
    echo "\n${RED}Test 17 Failed: File '1024bytes.txt' was not created 😓${NC}"
endif

# Clean up log file
rm -f 1024bytes.txt

echo "${BLUE}=====================================================================${NC}"

# Test 18: Request existing binary file of size exactly equal to 512 with no loss simulation
echo "${BLUE}=== Running Test 18: Request existing binary file of size exactly equal to 512 with no loss simulation ===${NC}"
./normalClient ${IP_ADDRESS} ${PORT} 512bytes.txt 2 >& /dev/null

# Check if 512bytes.txt exists and matches existing512bytes.txt
test -f 512bytes.txt
if ( $? == 0 ) then
    cmp -s 512bytes.txt existing512bytes.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 18 Passed: File '512bytes.txt' exists and matches 'existing512bytes.txt' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 18 Failed: File '512bytes.txt' does not match 'existing512bytes.txt' 😓${NC}"
    endif
else
    echo "\n${RED}Test 18 Failed: File '512bytes.txt' was not created 😓${NC}"
endif

# Clean up log file
rm -f 512bytes.txt

# Test 19: Request existing binary file of size exactly equal to 512 with loss simulation of last packet
echo "${BLUE}=== Running Test 19: Request existing binary file of size exactly equal to 512 with loss simulation of last packet ===${NC}"
./lossSimulation ${IP_ADDRESS} ${PORT} 512bytes.txt 2 2 >& /dev/null

# Check if 512bytes.txt exists and matches existing512bytes.txt
test -f 512bytes.txt
if ( $? == 0 ) then
    cmp -s 512bytes.txt existing512bytes.txt
    if ( $? == 0 ) then
        echo "\n${GREEN}Test 19 Passed: File '512bytes.txt' exists and matches 'existing512bytes.txt' 🎉${NC}"
        @ passed_tests++
    else
        echo "\n${RED}Test 19 Failed: File '512bytes.txt' does not match 'existing512bytes.txt' 😓${NC}"
    endif
else
    echo "\n${RED}Test 19 Failed: File '512bytes.txt' was not created 😓${NC}"
endif

# Clean up log file
rm -f 512bytes.txt

echo "${BLUE}=====================================================================${NC}"


echo "\n${BLUE}======================== All tests completed ========================${NC}\n"

if ( $passed_tests == $total_tests ) then
    echo "${GREEN}Passed ALL $passed_tests/$total_tests tests 🎉${NC}\n"
    echo "${GREEN}You're the real Mr MIT 🐐${NC}\n"
else if ( $passed_tests == 0 ) then
    echo "${RED}Passed NO tests 😢. Guess you're not Mr MIT...${NC}\n"
else
    echo "${RED}Passed only $passed_tests/$total_tests tests 😢. Guess you're not Mr MIT...${NC}\n"
endif

echo "${BLUE}=====================================================================${NC}"
