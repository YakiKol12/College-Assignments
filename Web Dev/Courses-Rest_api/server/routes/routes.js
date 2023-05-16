const express = require('express'),
    userRoutes = require('./users');

var router = express.Router();


router.get('/users', userRoutes.read_users);
router.post('/users', userRoutes.create_user);
router.put('/users/:id', userRoutes.update_user);
router.delete('/users/:id', userRoutes.delete_user);

module.exports = router;