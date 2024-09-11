## About

This repository contains bots for playing the popular board game Puerto Rico, written in C++.

**Puerto Rico** is a 3-5 player resource management game, similiar to "Castles of Burgundy" and "Settlers of Catan".

## Strategies / Algorithms used

The first, weaker bot Strategies use a classic minmax algorithm for exploring the game tree in combination with a human-made heuristic for evaluation the fitness of states. (Not yet implemented.)

This heuristic function can then potentially be modified with reinfocement learing or a genetic algorithm. The bots simulate a large number of games among each other, and evolve their heuristic function in ways that increase their winrate.
A neural network could also be trained, if required. And the minmax algorithm can be optimized with alpha-beta-pruning. This is all complicated by the fact that Puerto Rico is a game with more than two players. (Not yet implemented.)
Studying which buildings and moves have high "rewards" in the heuristic function could even help to teach humans how to player better (as long as the heuristic is reversible and not overly complex).

The stronger bots will use the **Monte Carlo Tree Search** algorithm to simulate random-runouts, allowing them to find strong moves without relying on any domain-specific evaluation function.
This method is used by famous bots such as AlphaGo and AlphaZero. The end goal is to make the bots unbeatable by normal humans. The MCTS algorithm can also be augmented by a strong heuristic to play even better. (Not yet implemented.)

Currently running main.cpp will simulate a random game of Puerto Rico played by 4 very bad bots who just make random legal moves. Information about every move and event is printed to the console.
Initially the goal was to implement and verify 100% of the game rules (which there are a lot of).

## Building the project

The project is currently easy to build by just executing something like:

> g++ main.cpp -o main.exe

This will change as soon as more .cpp files or dependencies are added. I'll make a CMake file when it becomes needed.

## Contributing

Feel free to take a look around and contribute. I'm especially interested in any bugs or edge cases in the game simulation engine. This will also become easier to test as soon as I implement playing versus the bots from the console window. (Human vs. Bots)
