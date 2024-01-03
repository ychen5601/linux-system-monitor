### CSCB09 Assignment 3 - System Monitoring Tool: Concurrency and Signals

**REDACTED**

By compiling and running this project in a Linux system, you will be able to print out information
with respect to this system in real time. There are four categories of information to be printed -
Memory usage, CPU usage, Information on logged in users, and System information in that order.

# Flags

There are various flags you can use to customize preferences for your output.

--system - you will only print system stats, cpu usage, memory usage.
--user - you will only print information on logged in users.

if both --system and --user are triggered, or if none are, then all stats will be printed.

--samples=N - there will be N number of pings of this program, updating information in real time.
--tdelay=N - between each ping, there will be a delay of N seconds.

If not specified, then the default values are samples=10, tdelay=1.

# Positional Arguments

these arguments can also be used as positional flags at the VERY START of command line arguments.
eg. ./mySystemStats 3 4 will default to samples=3, tdelay=4. If positional flags are used, they
must both be indicated - you cannot only input one number, as an error will be raised. Positional
arguments can also only appear at the very start - otherwise an error will be raised as well.

--graphics - prints out a graphical representation of each ping of CPU and memory usage. In CPU's
case, each bar represents 1%. In GPU's case, the change in usage from previous ping is drawn.
For example, +0.01 from previous = #, -0.01 = :

--sequential - prints output sequentially - All pings of CPU and memory will be completed before
user info and system info. User and system information will only be printed once. This is by
design - In outputs with a lot of samples, as well as with a lot of users, this information will
be redundant and cluttered. As a result, I decided to ping for this information only once.

If any other command line arguments are used, the system will not recognize it and an error will
be raised. Other than positional flags for tdelay and samples, they can be input in any order.
If positional arguments are raised, as well as --tdelay=N or --samples=N, an error will be raised
as well.
