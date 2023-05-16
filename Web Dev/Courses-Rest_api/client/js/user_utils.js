$(document).ready(function () {
   
    $("form[name='user_form']").validate({
        // Specify validation rules
        rules: {
          "name":{
            minlength: 5
          },
          "id_field": {
            required: true,
            digits: true,
            minlength: 6
          },
          "email":{
            "email":true
          }
        },
        // Specify validation error messages
        messages: {       
          name: "Your name must be at least 5 characters long",
          id_field:{
            digits:"Please enter only digits",
            minlength: "Your name must be at least 6 characters long"
          },
          email: "email structure is some@domain "
        }
      });

    // process the form
    $('#user_form').submit(function (event) {
        if(!$("#user_form").valid()) return;

        console.log("in submit");
        
        // process the form
        $.ajax({
            type: 'POST', // define the type of HTTP verb we want to use (POST for our form)
            url: '/users', // the url where we want to POST
            contentType: 'application/json',
            data: JSON.stringify({
                "name": $("#username").val(),
                "password": $("#password").val(),
                "profession": $("#profession").val(),
                "id": $("#id_field").val(),
                "email": $("#email").val(),
            }),
            processData: false,            
           // dataType: 'json', // what type of data do we expect back from the server
            encode: true,
            success: function( data, textStatus, jQxhr ){
                console.log(data);
                location.href = "/main";

            },
            error: function( jqXhr, textStatus, errorThrown ){
                console.log( errorThrown );
            }
        })
          
        // stop the form from submitting the normal way and refreshing the page
        event.preventDefault();
    });

});
