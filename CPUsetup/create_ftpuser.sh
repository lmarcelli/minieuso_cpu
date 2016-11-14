#!/usr/bin/expect
set timeout 10

spawn adduser -home /home/ftp-home/minieusouser minieusouser
expect {
    timeout { send_user "\nFailed to get password prompt\n"; exit 1 }
    eof { send_user "\nCommand failure\n"; exit 1 }
    "*assword"
}

send "minieusopass\r"

expect "*\$"
send "\r"
expect "*\$"
send "\r"
expect "*\$"

close 
