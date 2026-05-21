#!/usr/bin/env bats



# Helper function to start the server in the background
start_server() {
    ./dsh -s 0.0.0.0:1234 &
    SERVER_PID=$!
    sleep 1  # Give the server time to start
}

# Helper function to stop the server
stop_server() {
    kill $SERVER_PID
    wait $SERVER_PID 2>/dev/null
}

# Test 1: Check if the server starts and accepts connections
@test "Test server starts and accepts connections" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 2: Test remote command execution (echo)
@test "Test remote command execution (echo)" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
echo hello
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello"* ]]
}

# Test 3: Test remote command execution (ls)
@test "Test remote command execution (ls)" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
ls
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh"* ]]  # Assuming 'dsh' is in the current directory
}

# Test 4: Test remote command with pipes
@test "Test remote command with pipes" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
echo hello | wc -c
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"6"* ]]  # "hello" has 5 characters + newline = 6
}

# Test 5: Test remote command with redirection
@test "Test remote command with redirection" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
echo hello > remote_testfile.txt
cat remote_testfile.txt
rm remote_testfile.txt
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello"* ]]
}

# Test 6: Test remote command with background execution
@test "Test remote command with background execution" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
sleep 1 &
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 7: Test remote command with environment variables
@test "Test remote command with environment variables" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
export TEST_VAR=123
echo \$TEST_VAR
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"123"* ]]
}

# Test 8: Test remote command with multiple commands
@test "Test remote command with multiple commands" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
echo first
echo second
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"first"* ]]
    [[ "$output" == *"second"* ]]
}

# Test 9: Test remote command with error handling
@test "Test remote command with error handling" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
cd /nonexistent_directory
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]  # The shell should not crash
    [[ "$output" == *"No such file or directory"* ]]
}

# Test 10: Test remote command with stop-server
@test "Test remote command with stop-server" {
    start_server

    run ./dsh -c 127.0.0.1:1234 <<EOF
stop-server
exit
EOF

    stop_server

    # Assertions
    [ "$status" -eq 0 ]
}