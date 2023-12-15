const express = require('express'),
    coursesRoutes = require('./courses');
const router = express.Router();

router.get('/courses', coursesRoutes.read_courses);
router.get('/courses/:id', coursesRoutes.read_course);
router.post('/courses', coursesRoutes.create_course);
router.put('/courses/:id', coursesRoutes.update_course);
router.delete('/courses/:id', coursesRoutes.delete_course);
router.post('/courses/:id', coursesRoutes.add_student_to_course);
router.delete('/courses/:id/:student_id', coursesRoutes.delete_student_from_course);

router.get('/students', coursesRoutes.read_students);
router.post('/students', coursesRoutes.create_student);

module.exports = router;