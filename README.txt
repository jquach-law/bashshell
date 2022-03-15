A Bourne-Again Shell (BASH) replica that utilizes the Unix API to handle commands, 
multi-processes, I/O redirections, signals, and foreground/background executions

-------------------------------------------    
Regular method:
    1 Compile: gcc -std=gnu99 -Wall -g -o smallsh smallsh.c
    2 Run: smallsh
    3 Clean: rm -f smallsh
    
Makefile method:
    make, make run, make clean (compile, run, clean respectively)
    
-------------------------------------------
General syntax of a command line:
    : command [arg1 arg2 ...] [< input_file] [> output_file] [&]

where, [arg1 arg2 ...] takes multiple arguments
where, [< input_file] [> output_file] handles redirections
where, [&] to run command as background process

-------------------------------------------
Example:

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
 
 
 *Note: Some executions may differ
