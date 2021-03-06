# SpaceInvaders2.0
A real-time Space Invaders game off of the Arm Cortex-M3 with I/O Peripherals. Checkout main.c for the source code.

Space Invaders 2.0 is a single player game where the person playing has to “shoot” the incoming obstacles and destroy them before they reach the spaceship.

## I/O Peripheral Controls
The spaceship consists of a row of pixels on the bottom of the LCD, and is free to move in the horizontal direction controlled by user input from the potentiometer. The obstacles are arranged in a grid of 3x3 blocks on the centre of the screen that move down in the y-direction every time interval, and doing a sweep of the screen across the x-coordinate; autonomously controlled by the software on the microcontroller. The user must change the x-coordinate of the spaceship to align with the centre of the spaceship (where the laser is), and the x-coordinate of the block that is being targeted. Once the player presses the INT0 push button, the laser fires and instantly deletes the closest block with that x-coordinate (if there is any.) If the laser hits a block, the LEDs flash once, congratulating the player on their small win. If however, the blocks reach the bottom of the LCD screen, the LEDs continuously flash signalling a loss, until the user resets the microcontroller.

## TASKS

The system contains 3 tasks to handle the game. The first of these tasks handles the movement of the obstacles, and to check if the player has lost. This task will decrement the position of all available (not destroyed) blocks in the system every time interval. This task has the lowest priority as it does not need to interrupt any of the other tasks and has the longest time-frame to complete. However, if the position of any of the blocks reaches the zero level (aka the level of the spaceship), it will execute an infinite loop to signal the game is over to the player. 

The second task handles the movement of the shooter. This task is independent of the first task as the obstacles and shooter itself do not interact(only the laser from the shooter interacts with the obstacles.) As the shooter can only move in a line, this task will be responsible for using the potentiometer to map which pixel to move the shooter to. This task has a higher priority than Obstacle Movement task as it must respond properly to user input. 

The third task handles the shooting, deletion of the blocks, and managing the LEDs. This task has the highest priority as it needs to check the instantaneous coordinates of both the shooter and the obstacles. This task interacts with both of the other tasks. Once the push button is pressed, it checks the coordinates of the shooter, then check the coordinates of the obstacles, then delete the obstacle with a matching x-coordinate and lowest y-coordinate. If the deletion is successful, it will also send a signal to turn on the LED lights to indicate a successful shot. It then turns off the LED after a short time.
