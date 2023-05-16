const fs = require('fs');

// variables
const dataPath = './server/data/users.json';

// helper methods
const readFile = (callback, returnJson = false, filePath = dataPath, encoding = 'utf8') => {
    fs.readFile(filePath, encoding, (err, data) => {
            if (err) {
                console.log(err);
            }
            if (!data) data="{}";
            callback(returnJson ? JSON.parse(data) : data);
       });
};

const writeFile = (fileData, callback, filePath = dataPath, encoding = 'utf8') => {

        fs.writeFile(filePath, fileData, encoding, (err) => {
            if (err) {
                console.log(err);
            }

            callback();
        });
    };


module.exports = {
    //READ
    read_users: function (req, res) {
        fs.readFile(dataPath, 'utf8', (err, data) => {
            if (err) {
                console.log(err);
                res.sendStatus(500);                 
            }
            else
                res.send(!data? JSON.parse("{}") : JSON.parse(data));
        });
    },
  
    // CREATE
   create_user: function (req, res) {

        readFile(data => {

            // add the new user
            if (!req.body.id) return res.sendStatus(500);   
            data[req.body.id] = req.body;

            writeFile(JSON.stringify(data, null, 2), () => {
                res.status(200).send('new user added');
            });
        },
            true);
    },

    // UPDATE
    update_user: function (req, res) {

        readFile(data => {

            // add the new user
            const userId = req.params["id"];
            if (data[userId])
                data[userId] = req.body;
            else res.sendStatus(400);

            console.log("after if")

            writeFile(JSON.stringify(data, null, 2), () => {
                res.status(200).send(`users id:${userId} updated`);
            });
        },
            true);
    },
    // DELETE
    delete_user: function (req, res) {

        readFile(data => {

            // add the new user
            const userId = req.params["id"];
            delete data[userId];

            writeFile(JSON.stringify(data, null, 2), () => {
                res.status(200).send(`users id:${userId} removed`);
            });
        },
            true);
    }
};