All of our testing worked without any errors.
We did find, however, that connecting between different CSIL computers, some came back with an invalid host name error.
From further testing we found that csil-cpu6 and csil-cpu8 cannot be connected to.
Searching for clues we discovered that CPU6 and CPU8 are hosted in Burnaby while every other CPU is hosted in Surrey.
This was discovered through: "http://www.sfu.ca/computing/about/support/csil/unix/how-to-use-csil-linux-cpu-server.html"

Because of how the socket program is a simple one, firewalls are the main contributer to this problem. Leading to the fact that the program written can only be ran on computers that share the same local network.

S-talk program created by Justin Mateo and Raymond Cao
Inspiration of UDP send and UDP receive taken from Beej's Guide to Network Programming:
"https://beej.us/guide/bgnet/html/"
