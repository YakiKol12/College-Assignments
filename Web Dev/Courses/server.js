const express = require('express'),
    bodyParser = require('body-parser'),
    path = require('path'),
    fs = require('fs'),
    cors = require('cors'),
    routers = require('./server/routes/routes.js');
require('./server/db/mongoose.js');


const app=express();
const port = 3001;

app.use('/', express.static(path.join(__dirname, 'client/html/index.html')));
app.use('/list', express.static(path.join(__dirname, 'client/html/index.html')));
app.use('/add_course', express.static(path.join(__dirname, 'client/html/add_course_form.html')));
app.use('/add_student', express.static(path.join(__dirname, 'client/html/add_student_form.html')));
  

app.use('/js', express.static(path.join(__dirname, 'client/js')));
app.use('/css', express.static(path.join(__dirname, 'client/css')));

//restfull 
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

app.use('/', routers);

const server = app.listen(port, () => {
    console.log('listening on port %s...', server.address().port);
});