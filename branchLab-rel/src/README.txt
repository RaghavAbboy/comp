Raghav Abboy
A53081298
CSE 240A Project 1

Custom Predictor Description
--------------------------------------------------------

Paper: Piecewise Linear Branch Prediction
Author: Daniel A Jimenez

Link: http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=1431572

I chose to use the piecewise linear BP because I learnt that while the tournament predictor tries to choose the better of the local and global predictors, it does not take into consideration specific path based patterns that can be a stronger indicator of whether a branch is likely taken/not taken.

The piece wise linear BP uses a 3D array to determine the likelihood of a branch address being taken using the path of addresses that lead to a taken branch. This algorithm is explained in the paper.

I used this concept and implemented a piecewise linear predictor that sums up the weights associated with path of branches that likely contribute to the current branch address.