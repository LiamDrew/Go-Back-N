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

# # Test 12: Request existing binary file with window size 7, simulating loss of 3 random packets
# echo "${BLUE}=== Running Test 12: Request existing binary file with window size 7, simulating loss of 3 random packets ===${NC}"
# echo "${BLUE}=== ⚠️ This test takes some time because of the loss simulation ⚠️ ===${NC}"

# ./lossSimulation ${IP_ADDRESS} ${PORT} MrMIT.jpeg 7 2,7,23 >& /dev/null

# # Check if MrMIT.jpeg exists and matches existingMrMIT.jpeg
# test -f MrMIT.jpeg
# if ( $? == 0 ) then
#     cmp -s MrMIT.jpeg existingMrMIT.jpeg
#     if ( $? == 0 ) then
#         echo "\n${GREEN}Test 12 Passed: File 'MrMIT.jpeg' exists and matches 'existingMrMIT.jpeg' 🎉${NC}"
#         @ passed_tests++
#     else
#         echo "\n${RED}Test 12 Failed: File 'MrMIT.jpeg' does not match 'existingMrMIT.jpeg' 😓${NC}"
#     endif
# else
#     echo "\n${RED}Test 12 Failed: File 'MrMIT.jpeg' was not created 😓${NC}"
# endif

# rm -f MrMIT.jpeg
# echo "${BLUE}=====================================================================${NC}"


# BELOW: These tests revealed memory management issues

# # Test 6: Request existing binary file with window size 1
# echo "${BLUE}=== Running Test 6: Request existing binary file with window size 1 ===${NC}"
# ./normalClient ${IP_ADDRESS} ${PORT} play.jpg 1 >& /dev/null

# # Check if play.jpg exists and matches existingPlay.jpg
# test -f play.jpg
# if ( $? == 0 ) then
#     cmp -s play.jpg existingPlay.jpg
#     if ( $? == 0 ) then
#         echo "\n${GREEN}Test 6 Passed: File 'play.jpg' exists and matches 'existingPlay.jpg' 🎉${NC}"
#         @ passed_tests++
#     else
#         echo "\n${RED}Test 6 Failed: File 'play.jpg' does not match 'existingPlay.jpg' 😓${NC}"
#     endif
# else
#     echo "\n${RED}Test 6 Failed: File 'play.jpg' was not created 😓${NC}"
# endif

# rm -f play.jpg
# echo "${BLUE}=====================================================================${NC}"

# # Test 7: Request existing binary file with window size 9
# echo "${BLUE}=== Running Test 7: Request existing binary file with window size 9 ===${NC}"
# ./normalClient ${IP_ADDRESS} ${PORT} play.jpg 9 >& /dev/null

# # Check if play.jpg exists and matches existingPlay.jpg
# test -f play.jpg
# if ( $? == 0 ) then
#     cmp -s play.jpg existingPlay.jpg
#     if ( $? == 0 ) then
#         echo "\n${GREEN}Test 7 Passed: File 'play.jpg' exists and matches 'existingPlay.jpg' 🎉${NC}"
#         @ passed_tests++
#     else
#         echo "\n${RED}Test 7 Failed: File 'play.jpg' does not match 'existingPlay.jpg' 😓${NC}"
#     endif
# else
#     echo "\n${RED}Test 7 Failed: File 'play.jpg' was not created 😓${NC}"
# endif

# rm -f play.jpg
# echo "${BLUE}=====================================================================${NC}"