
#ifndef S_TALK_H
#define S_TALK_H

// Number of arguments when start s-talk via commandline:
// ./s-talk [local port number] [remote machine name] [remote port number]
// 1        2                   3                     4
#define S_TALK_CMD_NUM_ARGS 4
#define S_TALK_CMD_BAD_ARGUMENTS 126


/**
 * s_talk_starTalk(int numArgs, char *cmdLineArgs[])
 * Starts s-talk
 * Pass in directly the command line arguments used when invoking ./s-talk from command line
 */
void s_talk(int numArgs, char *cmdLineArgs[]);



#endif