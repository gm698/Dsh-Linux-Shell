# dsh-linux-shell

Custom Unix-style shell written in C featuring command parsing, process creation, piping, redirection, and remote shell communication using TCP sockets.

## Overview

`dsh` is a lightweight Linux shell developed in C as part of a systems programming project. The shell supports local command execution as well as a remote client/server mode that allows commands to be executed across a network connection using TCP sockets.

The project demonstrates low-level Linux programming concepts including:

- process management
- pipes and inter-process communication
- socket programming
- command parsing
- client/server architecture
- memory management
- system calls

The shell supports both:
- Local shell mode
- Remote shell mode (`client/server`)

---

## Features

### Local Shell
- Execute Linux commands
- Built-in commands:
  - `cd`
  - `exit`
  - `rc`
- Command parsing with argument handling
- Multi-command pipelines using `|`
- Process execution using `fork()` and `execvp()`
- Exit status tracking

### Remote Shell
- TCP client/server architecture
- Remote command execution
- Multi-command pipeline support over sockets
- Remote shell client and server modes
- EOF-based communication protocol
- Multiple client request handling

### Testing
- Automated tests using BATS
- Remote execution validation
- Pipe and command execution testing
- Error handling tests

---

## Technologies Used

- C
- Linux System Calls
- TCP/IP Socket Programming
- GCC
- BATS Testing Framework

---

## Project Structure

```text
dsh-linux-shell/
│
├── dsh_cli.c
├── dshlib.c
├── dshlib.h
├── rsh_cli.c
├── rsh_server.c
├── rshlib.h
├── assignment_tests.sh
├── student_tests.sh
├── makefile
└── questions.md
```

---

## How It Works

### Local Mode

Running the executable without arguments starts the shell locally:

```bash
./dsh
```

Example:

```bash
dsh4> ls
dsh4> echo hello
dsh4> pwd
```

---

### Remote Server Mode

Start the server:

```bash
./dsh -s
```

Or specify an interface and port:

```bash
./dsh -s -i 0.0.0.0 -p 1234
```

---

### Remote Client Mode

Connect to the server:

```bash
./dsh -c -i 127.0.0.1 -p 1234
```

Example remote execution:

```bash
dsh4> echo hello | wc -c
6
```

---

## Example Features

### Pipelining

```bash
echo hello | wc -c
```

### Built-In Commands

```bash
cd ..
rc
exit
```

### Remote Execution

```bash
./dsh -s
./dsh -c
```

---

## Testing

Automated testing is included using BATS scripts for:
- connection validation
- remote execution
- pipelines
- redirection
- background execution
- error handling

Run tests with:

```bash
bats student_tests.sh
```

---

## Concepts Demonstrated

- Linux process management
- `fork()` / `execvp()`
- TCP socket programming
- inter-process communication
- command parsing
- pipes and redirection
- client/server systems
- dynamic memory management
- network protocol design

---

## What I Learned

This project strengthened my understanding of operating systems and systems programming concepts by implementing a functional shell environment from scratch. I gained experience working with Linux system calls, process execution, TCP networking, and command parsing while also learning how shells manage pipelines and inter-process communication internally.

---

## Author

Maurice Murray  
Software Engineering Student at Drexel University
