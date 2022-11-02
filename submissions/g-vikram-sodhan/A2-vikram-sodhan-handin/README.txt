Program works as expected.


Unfortunately, I am getting some Memory Leaks that I could not resolve. I feel like for every malloc I had a corresponding free when consuming that item from the list, as well as destroying any mutex variables and threads. 

Outside of the memory leak it should work as expected from testing on logging into two separate csil machines.