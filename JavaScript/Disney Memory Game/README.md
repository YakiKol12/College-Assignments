# Disney Memory Game

## Author
Yaki Kol

## Getting Started
To play the game, simply go to [Yaki's game](http://yakovko.mysoft.jce.ac.il/ex1/) in a web browser.


## Prerequisites
There are no prerequisites to run the game. However, you will need a web browser that supports HTML, CSS, and JavaScript.

## Files
index.html: The HTML file for the game interface.<br>
styles.css: The CSS file for styling the game interface.<br>
script.js: The JavaScript file that contains the game logic.

## How to Play
To play the game, simply open the index.html file in your browser.<br>
Click on "New Game" button to start a new game. Once the game has started, click on any two cards to flip them over.<br>
If the cards match, information about the character will be displayed, and the cards will stay face up. If they do not match, they will flip back over.<br>
Continue flipping cards until all pairs have been matched.<br>
Once all pairs have been matched, the player wins the game.<br><br>

The player can also start a new game by clicking on the "New Game" button at any time.

## HTTP Requests
In this program im using ajax mechanism a few times during the games lifetime.<br>

First call is used to reach an api providing me with the information i need to create the game cards, the characters and their images
as well as information about their apearences in media (movies, tv shows etc).<br>

Second call is to the games hosting server to recieve the current time when the game beggins.<br>

Third call is when the game is finished (player won) and it is also to the games hosting server to recieve the current time.
The two calls are used to calculate the time it took to the player to win the game.
