# CPSC 351 - Dining Philosophers Problem

## Overview
The program ensures that there is no deadlock or starvation for any of the philosophers sitting at the table, when they attempt to eat by picking up the forks from their left and right side.

#### Compile
g++ -pthread -Wall -std=c++11 main.cpp -o Exec

#### Run Application
./Exec&nbsp;&nbsp;\<# of philosophers\>&nbsp;&nbsp;<# of times each philosopher attempts to eat\>
