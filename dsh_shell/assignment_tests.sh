#!/usr/bin/env bats



# Test 1: Check if the server accepts connections
@test "Test server accepts connections" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 2: Test remote command execution (echo)
@test "Test remote command execution (echo)" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
echo hello
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>hellodsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 3: Test remote command execution (ls)
@test "Test remote command execution (ls)" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
ls
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>dshdshlib.cdsh_cli.cdshlib.hrsh_cli.crsh_server.crshlib.hstudent_tests.shdsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 4: Test remote command with pipes
@test "Test remote command with pipes" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
echo hello | wc -c
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>6dsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 5: Test remote command with redirection
@test "Test remote command with redirection" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
echo hello > remote_testfile.txt
cat remote_testfile.txt
rm remote_testfile.txt
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>hellodsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 6: Test remote command with background execution
@test "Test remote command with background execution" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
sleep 1 &
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 7: Test remote command with environment variables
@test "Test remote command with environment variables" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
export TEST_VAR=123
echo \$TEST_VAR
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>123dsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 8: Test remote command with multiple commands
@test "Test remote command with multiple commands" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
echo first
echo second
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>firstdsh4>seconddsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 9: Test remote command with error handling
@test "Test remote command with error handling" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
cd /nonexistent_directory
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>Nosuchfileordirectorydsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 10: Test remote command with stop-server
@test "Test remote command with stop-server" {
    run ./dsh -c 127.0.0.1:1234 <<EOF
stop-server
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>cmdloopreturned0"

    # Debugging output
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}