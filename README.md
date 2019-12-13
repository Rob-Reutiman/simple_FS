# Project 06: Simple File System

This is [Project 06] of [CSE.30341.FA19].

## Members

1. Rob Reutiman (rreutima@nd.edu)

## Brainstorming

## Errata

I am terribly confused as to what is wrong with my project. In fs.c on line 656, I iterate through 
the list of free_blocks. The while loop should end if the comparison is zero, but it keeps going 
for no reason. I print the comparison to stdout and can not determine how what I am doing could possibly
be wrong based on the values being printed. The while loop is simply ignoring the logic I put in.

Everything else works really well - this error was really annoying so I gave up after spending a quick 
18+ hours on this project.

> Describe any known errors, bugs, or deviations from the requirements.

[Project 06]:       https://www3.nd.edu/~pbui/teaching/cse.30341.fa19/project06.html
[CSE.30341.FA19]:   https://www3.nd.edu/~pbui/teaching/cse.30341.fa19/
