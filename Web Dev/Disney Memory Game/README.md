# Disney Memory Game

### Author: Yaki Kol

## Table of Contents

-   [Getting Started](#getting-started)
-   [Prerequisites](#Prerequisites)
-   [Files](#Files)
-   [How to Play](#how-to-play)
-   [HTTP Requests](#http-requests)

## Getting Started
To play the game, simply go to [Yaki's game](http://yakovko.mysoft.jce.ac.il/ex1/) in a web browser.


## Prerequisites
There are no prerequisites to run the game. However, you will need a web browser that supports HTML, CSS, and JavaScript.

## Files
- index.html: The HTML file for the game interface.<br>
- styles.css: The CSS file for styling the game interface.<br>
- script.js: The JavaScript file that contains the game logic.

## How to Play
1. Click on the `New Game` button to start the game.
2. Flip over two cards by clicking on them.
3. If the cards match, a popup window with the character's details will appear.
4. If the cards do not match, a `Try Again` banner will appear at the top of the screen and the cards will unflip after a short period of time.
5. Repeat steps 2-4 until all pairs have been found.
6. Once all pairs have been found, an alert will pop up indicating that you have won and how much time has passed.

The player can also start a new game by clicking on the "New Game" button at any time.

## HTTP Requests
In this program im using ajax mechanism a few times during the games lifetime.<br>

First call is used to reach an api providing me with the information i need to create the game cards, the characters and their images
as well as information about their apearences in media (movies, tv shows etc).<br>

Second call is to the games hosting server to recieve the current time when the game beggins.<br>

Third call is when the game is finished (player won) and it is also to the games hosting server to recieve the current time.
The two calls are used to calculate the time it took to the player to win the game.
