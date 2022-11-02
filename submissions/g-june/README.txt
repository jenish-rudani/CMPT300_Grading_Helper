!READ ME!
Assignment 2: s-talk 

By: June (Jun Hee) Lee
----------------------------------------

1. Due to not completely fleshed out code in Assignment 1, to prevent errors,
    when the two Lists, sendArray and receiveArray were initialized, 'blank' variables were pushed so the arrays can never be null.
    

2. To make the UI look prettier, I added "From # ->" and "To # ->" tags into the program.
    However, as the cursor position shifts whenever messages are recieved,
    the first "To # ->" message after every message receive is missing.
    
3. The project description didn't specify whether the messages should be received live
    or after userInput is collected. As such, although it would make more sense to attach
    a semaphore to fgets() as it manipulates stdin/stdout, I left it out to make the 
    message retrival real-time.
    
----------------------------------------   
    
A lot of the socket methodalogy/implementation was provided through the class given
resource: https://beej.us/guide/bgnet/

