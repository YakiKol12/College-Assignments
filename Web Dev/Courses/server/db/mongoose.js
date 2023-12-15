const mongoose = require('mongoose')

// mongoose.connect('mongodb://127.0.0.1:27017/Ex5').then(() => {
//     console.log('Connected to MongoDB');    
//   }).catch((error) => {
//     console.error('Error connecting to MongoDB:', error);
// });

mongoose.connect('mongodb://127.0.0.1:27017/Ex5',{ useNewUrlParser: true});