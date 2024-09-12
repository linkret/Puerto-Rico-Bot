## About

This repository contains bots for playing the popular board game **Puerto Rico**, written in C++.

Puerto Rico is a 3-5 player resource management game, similiar to "Castles of Burgundy" and "Settlers of Catan".

The rulebook can be found here: https://www.riograndegames.com/wp-content/uploads/2013/02/Puerto-Rico-Rules.pdf

![alt text](https://i0.wp.com/boardgamedragons.com/wp-content/uploads/2012/04/IMG_23301.jpg)

## Strategies / Algorithms used

The first, weaker bot Strategies use a classic minmax algorithm (for more than 2 players, it's called the maxN algorithm) for exploring the game tree in combination with a human-made heuristic for evaluating the fitness of gamestates. (Not yet implemented.)

This heuristic function can then potentially be modified with reinfocement learing or a genetic algorithm. The bots simulate a large number of games among each other, and evolve their heuristic function to increase their winrate.
A neural network could also be trained, if required. The minmax algorithm can then be sped up using shallow pruning. (Alpha-beta-pruning does not generalize to games with more than 2 players.) (Not yet implemented.)

Studying which buildings and moves have high "rewards" in the heuristic function could even help to teach humans how to player better (as long as the heuristic is reversible and not overly complex).

The stronger bots will use the **Monte Carlo Tree Search** algorithm to simulate random-runouts, allowing them to find strong moves without relying on any domain-specific evaluation function.
This method is used by famous bots such as AlphaGo and AlphaZero. The MCTS algorithm can also be augmented by a strong heuristic to play even better. (Not yet implemented.)

Read more about MTCS here: https://en.wikipedia.org/wiki/Monte_Carlo_tree_search

Currently running main.cpp will simulate a random game of Puerto Rico played by 4 very bad bots who just make random legal moves. Information about every move and event is printed to the console.
Initially the goal was to implement and verify 100% of the game rules (which there are a lot of).

The end goal is to make the bots unbeatable by normal humans!

## Building the project

The project is currently easy to build by just executing something like:

> g++ *.cpp -o main.exe

This will change as soon as more .cpp files or dependencies are added. I'll make a CMake file when it becomes needed. My g++ build command is also included in `.vscode/tasks.json`.

## Running the project

> main.exe

Running the executable will produce output that looks like this:

```
Seed: 1726086293
Player 0 chose Prospector and got 1 doubloon
Player 1 chose Builder:
Player 1 built a Construction Hut for 1 doubloons
Player 2 built a Small Market for 1 doubloons
Player 3 built a Small Indigo Plant for 1 doubloons
Player 0 built a Small Market for 1 doubloons
Player 2 chose Mayor:
Player 2 took 1 extra Colonist from the supply
...
Total Round count: 14
Player 0 score: 17
Player 1 score: 11
Player 2 score: 20
Player 3 score: 19
Player 2 is the winner!
```

## Contributing

Feel free to take a look around and contribute. I'm especially interested in any bugs or edge cases in the game simulation engine. This will also become easier to test as soon as I implement playing versus the bots from the console window. (Human vs. Computer support)
