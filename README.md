# ME384-kilobots

This repository contains code for the swarm robotics project for ME384 Mechatronic Systems at Bilkent University. This project uses the [Kilobotics platform](https://www.kilobotics.com/ "Kilobotics") to complete a swarm intelligence task. The task chosen is a rudimentary implementation of the classic snake game.

The final demonstration algorithm is "snake.c". At program start, a single kilobot is designated as the "head", and will move forward until it approaches another kilobot. At this point, leadership is transferred forward, and the old head will follow the new head. The kilobots will continue to move forward, and this process is repeated as the snake "eats" other kilobots and grows in length.

All code was compiled to hex using the online editor found [here](https://www.kilobotics.com/editor "Kilobotics editor").
