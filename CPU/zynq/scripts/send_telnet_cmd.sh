#!/usr/bin/expect
spawn telnet 192.168.7.10
expect "*$*"
send "acq start\r"
expect "*$*"
sleep 1
send "exit\r"
