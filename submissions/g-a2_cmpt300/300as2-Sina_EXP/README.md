## CMPT300 Assignment 2 Submission.

This program connects two endpoints using sockets and transmits using the UDP protocol.
It can be run on any linux system on the same network

## To run program

make clean

make the program

./s-talk [local port] [remote server] [remote port]

#### [local port] is your local endpoint

#### [remote server] is the name of your remote computer

#### [remote port] is the port you wish to connect to on the remote computer

#### Note: port number must be larger than 1024, anything below is reserved

Sources:  
During development we used the tutorials on the coursys page, <br />
https://beej.us/guide/bgnet/html/ <br />
https://coursys.sfu.ca/2022fa-cmpt-300-d2/pages/tut_sockets/view <br />
https://coursys.sfu.ca/2022fa-cmpt-300-d2/pages/tut_pthreads/view <br />
And Dr.Brian's workshop on Sockets and Threads.

