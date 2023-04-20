
class Card {
    constructor(character) {
        this.character = character;
        this.card_div = null;
        this.flipped = false;
        this.matched = false;
    }

    setCardDiv(card_div) {
        this.card_div = card_div;
        this.card_div.on('click', () => {
            if(game_state.second_flip === false && this.flipped === false && this.matched === false) {
                this.flip();
                gameLogic(this);
            }
        });
    }

    flip() {
        if(this.flipped === false && this.matched === false) {
            this.card_div.addClass('flipped');
            this.flipped = true;
        }
    }

    unflip() {
        if(this.flipped === true && this.matched === false) {
            this.card_div.removeClass('flipped');
            this.flipped = false;
        }
    }
}

let api_url = 'https://api.disneyapi.dev/characters';

let game_state = {
    starting_time: null,
    first_flip: false,
    second_flip: false,
    first_card: null,
    second_card: null,
    matches: 0,
    moves: 0
};
let deck = [];


// create game function
function createGame() {
    getTime();
    
    // get characters from api
    let data = getData(api_url);
    
    // get ten random characters
    let characters = [];
    for (let i = 0; i < 10; i++) {
        let random = Math.floor(Math.random() * data.length);
        characters.push(data[random]);
        data.splice(random, 1);
    }

    // create a board
    createBoard(characters);

}

// create board function
function createBoard(characters) {
    for (let i = 0; i < (characters.length); i++) {
        let card1 = new Card(characters[i]);
        let card2 = new Card(characters[i]);
        deck.push(card1);
        deck.push(card2);
    }
    // shuffle the deck
    deck.sort(() => Math.random() - 0.5);

    // create a board
    let board = $('#board');
    for (let i = 0; i < deck.length; i++) {
        let card = $('<div>').addClass('card');
        let front = $('<div>').addClass('front');
        let front_title = $('<h1>').text('?');
        let back = $('<div>').addClass('back');
        let img = $('<img>').attr('src', deck[i].character.imageUrl);
        let name = $('<p>').text(deck[i].character.name);
        name.addClass('char-name');
        front.append(front_title);
        back.append(img);
        back.append(name);
        card.append(front);
        card.append(back);
        board.append(card);
        // add card to deck
        deck[i].setCardDiv(card);
    }
}

// get data from api
function getData(url) {
    let data = null;
    $.ajax({
        url: url,
        dataType: 'json',
        async: false,
        success: function (response) {
            if(response.nextPage === null || response.nextPage === undefined || response.nextPage === "" || response.nextPage === '') {
                api_url = 'https://api.disneyapi.dev/characters';
            }
            else {
                api_url = response.nextPage;
            }
            data = response.data;
        }, 
        error: function (error) {
            console.log(error);
        }
    });
    return data;
}

// game logic function
function gameLogic(card) {
    if(game_state.first_flip === false) {
        game_state.first_flip = true;
        game_state.first_card = card;
    }
    else {
        game_state.second_flip = true;
        game_state.second_card = card;

        // check if cards match
        if(game_state.first_card.character.name === game_state.second_card.character.name) {
            game_state.first_card.matched = true;
            game_state.second_card.matched = true;
            game_state.matches++;

            $('.card').css('pointer-events', 'none');
            $('#match-screen').addClass('show');
            $('#board').addClass('blur');
            $('#character-name').text(game_state.first_card.character.name);
            $('#movies-info').text(game_state.first_card.character.films);
            $('#TV-shows-info').text(game_state.first_card.character.tvShows);
            $('#games-info').text(game_state.first_card.character.videoGames);
            $('#attractions-info').text(game_state.first_card.character.parkAttractions);
        }
        else {
            // disable clicking on other cards until both cards are flipped back
            $('.card').css('pointer-events', 'none');
            // alert for no matchs
            $('#try-again').addClass('show');
            setTimeout(function() {
                game_state.first_card.unflip();
                game_state.second_card.unflip();
                game_state.first_card = null;
                game_state.second_card = null;
                

                // re-enable clicking on other cards after flipping back
                $('.card').css('pointer-events', 'auto');
                $('#try-again').removeClass('show');
            }, 1500);
        }
        game_state.first_flip = false;
        game_state.second_flip = false;
    }
}

// get time function
function getTime() {
    $.ajax({
        url: 'get_current_time.php',
        success: function(response) {
            if(game_state.starting_time === null) {
                game_state.starting_time = parseInt(response);
            }
            else {
                let current_time = parseInt(response);
                let time = current_time - game_state.starting_time;
                let minutes = Math.floor(time / 60);
                if(minutes < 10) {
                    minutes = '0' + minutes;
                }
                let seconds = time % 60;
                if(seconds < 10) {
                    seconds = '0' + seconds;
                }
                $('#time').text(minutes + ':' + seconds + 's');
            }
        },
        error: function(error) {
            console.log(error);           
        }
    });
}

$(document).ready(function() {
    // create new game button
    $('.new-game').click( () => {
        if($('#match-screen').hasClass('show')) {
            $('#match-screen').removeClass('show');
        }
        if($('#game-over').hasClass('show')) {
            $('#game-over').removeClass('show');
        }
        // remove cards from board
        $('#board').css('visibility', 'visible');
        $('#board').empty();
        // reset game state
        game_state.starting_time = null;
        game_state.first_flip = false;
        game_state.second_flip = false;
        game_state.first_card = null;
        game_state.second_card = null;
        game_state.matches = 0;
        game_state.moves = 0;
        // reset deck
        deck = [];
        // create new game
        createGame();
    });

    $('#continue-button').click( () => {
        $('#match-screen').removeClass('show');
        $('#board').removeClass('blur');

        if(game_state.matches === 10) {
            $('#board').css('visibility', 'hidden');
            $('#game-over').addClass('show');
            getTime();
        }
        else {
            $('.card').css('pointer-events', 'auto');
        }
    });
});
  