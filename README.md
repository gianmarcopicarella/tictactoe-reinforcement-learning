# Reinforcement learning applied to Tic-Tac-Toe
The following application applies at its most basic level some of the core concept of reinforcement learning in order to train a policy capable of playing the game of tic tac toe autonomously. I used the following project as a starting point to dive into the reinforcement learning field, I hope it will help others too.
## Project structure
I tried to keep separated the core reinforcement learning logic and the domain-specific logic concerning tic-tac-toe. In this way it should be easier to understand the code.

The learning agent is based on an epsilon-greedy QLearning algorithm. The epsilon value represent the probability to select a random, or exploratory, move. Otherwise it selects all the maximum scored moves, if more than one, and randomly picks one among them.

At the end of each game, or episode, the agent updates its policy iterating the gameplay history of moves so that the final reward is spreaded from leaf nodes towards the root. Repeating the following logic with the proper learning parameters many times guarantees that the estimated policy will eventually converge to the optimal one.

The opponents available are two: random or epsilonOptimal
A random opponent simply selects a random move everytime.
An epsilonOptimal opponent selects a random move with probability epsilon and uses the minimax algorithm to find the best move all the other times.

Why using an epsilon optimal opponent? Because in this way the agent is also capable of learning to win, otherwise we would have all draws even if playing against a random player.

## Running the application
### Training an agent

### Testing an agent

### Plotting cumulative reward function

### Plotting Win\Draw\Lose histograms
