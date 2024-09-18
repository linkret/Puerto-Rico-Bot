## About

This repository contains bots for playing the popular board game **Puerto Rico**, written in C++. You can play against them yourself, or watch them fight each other!

Puerto Rico is a 3-5 player resource management game, similiar to "Castles of Burgundy", "Citadels" and "Settlers of Catan".

The rulebook can be found here: https://www.riograndegames.com/wp-content/uploads/2013/02/Puerto-Rico-Rules.pdf

![alt text](https://i0.wp.com/boardgamedragons.com/wp-content/uploads/2012/04/IMG_23301.jpg)

## Strategies / Algorithms used

The first, weaker bot Strategies use a classic minmax algorithm (for more than 2 players, it's called the maxN algorithm) for exploring the game tree in combination with a human-made heuristic for evaluating the fitness of gamestates.

This heuristic function can then potentially be modified with reinfocement learning or a genetic algorithm. The bots simulate a large number of games among each other, and evolve their heuristic function to increase their winrates.
A neural network could also be trained, if required. The minmax algorithm can then be sped up using shallow pruning. (Alpha-beta-pruning does not generalize to games with more than 2 players.)

Studying which buildings and moves have high "rewards" in the heuristic function could even help to teach humans how to play better (as long as the heuristic is reversible and not overly complex).

The stronger bots use the **Monte Carlo Tree Search** algorithm to simulate random-runouts, allowing them to find strong moves without relying on any domain-specific evaluation function.
This method is used by famous bots such as AlphaGo and AlphaZero. The MCTS algorithm can also be augmented by a strong heuristic to play even better.

Read more about MTCS here: https://en.wikipedia.org/wiki/Monte_Carlo_tree_search

The reinforcement learning / heuristic evolution hasn't been implemented yet.

Currently running `main.exe` will let you play against the computer - which is configured to use the MCTS algorithm (with search set to only 500 iterations). When I optimize the code to run faster, I'll up this iteration count. There's also an idea to give the bots a fixed amount of time (like 1000ms) for each move, and just output the best found move regardless of iteration count. This would also work for maxN() algorithm with iterative deepening!

The end goal is to make the bots unbeatable by normal humans. The computer is already able to often beat me, and I consider myself a decent enough player.

## Building the project

The project can be built manually by executing something like:

> g++ src/* -o main.exe -std=c++17 -I include -Wall -O2

Alternatively, you can use CMake as per usual:

```
mkdir build
cd build
cmake ..
cmake --build .
cd ..
```

## Running the project

> ./main.exe

Running the executable will let you play against the computer. The game itself looks like this:

```
Seed: 1726607175
Player count:
3 players
4 players
5 players
Enter the number of the players you want to have in your game: 4
What position do you want to play?
1: Position 1
2: Position 2
3: Position 3
4: Position 4
5: Random position
Enter the number of the player position you want to play as: 2
Game starting...

Player 0 chose role: Craftsman
Player 0 got:
Player 1 got:
Player 2 got:
Player 3 got:

Choose an available role:
1: Mayor
X: Craftsman (taken)
2: Trader
3: Settler
4: Builder
5: Captain
6: Prospector
G or B: Print the entire current game state (G) or just your own board (B)
Enter the number of the role you want to choose: 4
You have 3 doubloons
Choose a building to build:
1: Small Indigo Plant (0 doubloons)
2: Small Sugar Mill (1 doubloons)
3: Small Market (0 doubloons)
4: Hacienda (1 doubloons)
5: Construction Hut (1 doubloons)
6: Small Warehouse (2 doubloons)
7: Large Indigo Plant (2 doubloons)
8: Large Sugar Mill (3 doubloons)
9: Hospice (3 doubloons)
10: None (0 doubloons)
G or B: Print the entire current game state (G) or just your own board (B)
Enter the number of the building you want to build: 3
Player 1 chose role: Builder
Player 1 built a Small Market for 0 doubloons
Player 2 built a Small Indigo Plant for 1 doubloons
Player 3 built a Small Indigo Plant for 1 doubloons
Player 0 built a Small Indigo Plant for 1 doubloons

Player 2 chose role: Mayor
Player 2 took 1 extra Colonist from the supply
Player 2 distributed Colonists:
Buildings: Small Indigo Plant 1/1,
Plantations: Corn 1/1,
Player 3 distributed Colonists:
Buildings: Small Indigo Plant 0/1,
Plantations: Corn 1/1,
Player 0 distributed Colonists:
Buildings: Small Indigo Plant 0/1,
Plantations: Indigo 1/1, 
Your current colonist distribution:
Extra Colonists: 1
Buildings: Small Market 0/1,
Plantations: Indigo 0/1,
Choose whether to add or remove colonists:
1: Add colonists
2: Remove colonists
3: Done
Enter the number of the action you want to perform:
...
```

## Contributing

Feel free to take a look around and contribute. I'm especially interested in any bugs or edge cases in the game simulation engine. Please try playing against my bots and tell me what you think!
