# Bourne-Again-N-Again Shell

A BASH replica that uses the Unix API to handle commands, multi-processes, I/O redirections, signals, and foreground/background executions

## Installation

Libraries:
```c
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
```

Compile & Execute in bash:
```bash
gcc -std=gnu99 -Wall -g -o smallsh smallsh.c
smallsh
```

## Usage

```bash
$ gcc -std=gnu99 -Wall -g -o smallsh smallsh.c
$ smallsh
: ls > junk
: cat junk
junk
smallsh
smallsh.c
: wc < junk > junk2
: wc < junk
       3       3      23
: ps
  PID TTY          TIME CMD
 4923 pts/0    00:00:00 sleep
 4564 pts/0    00:00:03 bash
 4867 pts/0    00:01:32 smallsh
 4927 pts/0    00:00:00 ps
: exit
$
```

Command line syntax:

* : command [arg1 arg2 ...] [< input_file] [> output_file] [&]

  * [arg1 arg2 ...] takes multiple arguments
  * [< input_file] [> output_file] handles redirections
  * [&] to run command as background process
