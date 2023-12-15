const fs = require('fs');
const express = require('express');
const Course = require('../models/courses');
const Student = require('../models/students');


// variables
const dataPath = './server/data/courses.json';

module.exports = {

    // READ COURSES
    read_courses: function (req, res) {
        Course.find().then(courses => {
            res.send(courses)
        }).catch(e => res.status(500).send())
    },

    // READ COURSE
    read_course: function (req, res) {
        Course.findOne({course_id: req.params.id}).then(course => {
            if (!course) {
                return res.status(404).send()
            }
            res.send(course)
        }).catch(e => res.status(500).send())
    },

    // CREATE COURSE
    create_course: function (req, res) {
        const course = new Course(req.body)
        course.save().then(course => {
            res.redirect('/list');
        }).catch(e => { 
            res.status(400).send('<script>alert("Error occurred while creating a course, make sure that course not already exists."); window.location.href = "/list";</script>');
        });
    },

    // UPDATE COURSE
    update_course: function (req, res) {
        // get course id
        const course_id = req.params.id;

        // get course from database
        Course.findOne({course_id: course_id}).then(course => {
            if (!course) {
                return res.status(404).send()
            }
            else {
                // update course
                course.name = req.body.name ? req.body.name : course.name;
                course.lecturer = req.body.lecturer ? req.body.lecturer : course.lecturer;
                course.start_date = req.body.start_date ? req.body.start_date : course.start_date;
                course.end_date = req.body.end_date ? req.body.end_date : course.end_date;
                course.save().then(course => {
                    res.status(200).send(course)
                }).catch(e => { 
                    res.status(400).send('<script>alert("Error occurred while updating the course."); window.location.href = "/list";</script>')
                });
            }
        }
        ).catch(e => res.status(400).send(e))
    },

    // DELETE COURSE
    delete_course: function (req, res) {
        Course.deleteOne({course_id: req.params.id}).then(course => {
            if (course.deletedCount > 0) {
                res.status(200).send();
              } else {
                res.status(404).send();
              }
        }).catch(e => {res.status(500).send(e)});
    },

    // CREATE STUDENT
    create_student: function (req, res) {
        const student = new Student(req.body)
        student.save().then(student => {
            res.redirect('/list');
        }).catch(e => {
            res.status(400).send('<script>alert("Error occurred while creating the student, make sure that student not already exists."); window.location.href = "/list";</script>');  
        });
    },

    // READ STUDENTS
    read_students: function (req, res) {
        Student.find().then(students => {
            res.send(students)
        }).catch(e => res.status(500).send())
    },

    // ADD STUDENT TO COURSE
    add_student_to_course: function (req, res) {
        // get course id
        const course_id = req.params.id;

        // get course from database
        Course.findOne({course_id: course_id}).then(course => {
            if (!course) {
                return res.status(404).send()
            }
            else {
                // get student id
                const student_id = req.body.student_id;

                // get student grade
                const grade = req.body.grade;

                // add student to course
                course.grades.push({student: student_id, grade: grade});
                course.save().then(course => {
                    res.status(200).send(course)
                }).catch(e => {
                    res.status(400).send(e)
                });
            }
        }
        ).catch(e => res.status(400).send(e))
    },

    // DELETE STUDENT FROM COURSE
    delete_student_from_course: function (req, res) {
        // get course id
        const course_id = req.params.id;

        // get course from database
        Course.findOne({course_id: course_id}).then(course => {
            if (!course) {
                return res.status(404).send()
            }
            else {
                // get student id
                const student_id = req.params.student_id;



                // delete student from course
                course.grades = course.grades.filter(grade => grade.student != student_id);
                course.save().then(course => {
                    res.status(200).send(course)
                }).catch(e => { 
                    res.status(400).send(e)
                });
            }
        }
        ).catch(e => res.status(400).send(e))
    },

    
    

//     //READ COURSES
//     read_courses: function (req, res) {
//         fs.readFile(dataPath, (err, data) => {
//             if (err) {
//                 console.log(err);
//                 res.sendStatus(500);                 
//             }
//             else
//                 res.send(!data ? JSON.parse("{}") : JSON.parse(data));
//         });
//     },
    
//     // READ COURSE
//     read_course: function (req, res) {
//         // get course id
//         readFile(data => {
//             // if the course id is not in the list return 400
//             if (!data[req.params.id])
//                 return res.status(404).send(`Course id:${req.params.id} not found`);
//             res.status(200).send(data[req.params.id]);
//         }, true)
//     },

//     // CREATE COURSE
//    create_course: function (req, res) {

//         readFile(data => {

//             // check for missing id in course
//             if (!req.body.id) return res.send('Bad request - id is required');

//             // check for duplicate id
//             if(req.body.id in data) return res.send('Bad request - id already exists');

//             // check for missing fields in course
//             if(!req.body.name || !req.body.lecturer || !req.body.start_date ||
//                 !req.body.end_date) {
//                     return res.send('Bad request: missing fields - name, lecturer, start_date, end_date');
//             }

//             // check for extra fields in course
//             if(Object.keys(req.body).length > 7) {
//                 return res.send('Bad request: too many fields');
//             }    

//             // if students is not empty, check for missing fields in student
//             if(req.body.students) {
//                 let students = JSON.parse(req.body.students);
//                 if (Object.keys(students).length > 0) {
//                     const studentIds = Object.keys(students);
//                     for (const studentId of studentIds) {
//                         const student = students[studentId];
//                         if (!student.id || !student.firstname || !student.surname || !student.picture || !student.grade) {
//                             return res.send(`Missing one or more fields for a student: ${studentId}
//                                 ${student.id} ${student.firstname} ${student.surname} ${student.picture} ${student.grade}`);
//                         }
        
//                         // check that grade is a valid number
//                         if (isNaN(student.grade) || student.grade < 0 || student.grade > 100) {
//                             return res.send(`Bad request - grade must be a valid grade number - ${student.firstname} , ${student.grade}`);
//                         }
//                     }
//                 }
//                 req.body.students = students;
//             }
//             else
//                 req.body.students = {};

//             // add the new course
//             data[req.body.id] = req.body;
//             writeFile(JSON.stringify(data, null, 2), () => {
//                 //TODO: redirect the user to the list page
//                 res.redirect('/list');
//             });
            

//         }, true);
//     },

//     // UPDATE COURSE
//     update_course: function (req, res) {
//         readFile(data => {
//             // get course id
//             const course_id = req.params.id;

//             // check if course exists
//             if (!data[course_id]) return res.sendStatus(400).send(`Bad request - ${course_id} does not exist`);

//             // update the course
//             if(req.body.name) data[course_id].name = req.body.name;
//             if(req.body.lecturer) data[course_id].lecturer = req.body.lecturer;
//             if(req.body.start_date) data[course_id].start_date = req.body.start_date;
//             if(req.body.end_date) data[course_id].end_date = req.body.end_date;





//             writeFile(JSON.stringify(data, null, 2), () => {
//                 res.status(200).send(`users id:${course_id} updated`);
//             });
//         }, true);
//     },

//     // DELETE COURSE
//     delete_course: function (req, res) {

//         readFile(data => {
//             // check for missing id in course
//             if (!req.params.id) return res.sendStatus(400).send('Bad request - course id is required');

//             // add the new user
//             const course_id = req.params.id;

//             if (!data[course_id]) return res.sendStatus(404).send(`Bad request - ${course_id} not found`);

//             delete data[course_id];

//             writeFile(JSON.stringify(data, null, 2), () => {
//                 res.status(200).send(`users id:${course_id} removed`);
//             });
//         },
//             true);
//     },

//     // ADD STUDENT TO COURSE
//     add_student_to_course: function (req, res) {

//         readFile(data => {
//             // get course id
//             const course_id = req.params.id;
//             const student_id = req.body.id;

//             // check if course exists
//             if (!data[course_id]) return res.send(`Bad request - ${course_id} not found`);

//             // check if student id exists
//             if (!student_id) return res.send('Bad request - student id is required');

//             // check if student id exists in course
//             if (student_id in data[course_id].students) return res.send(`Bad request - ${student_id} already exists in course`);

//             // check for missing fields in student
//             if (!req.body.firstname || !req.body.surname || !req.body.picture || !req.body.grade) {
//                 return res.send('Missing one or more fields for a student');
//             }

//             // check that grade is a valid number
//             if (isNaN(req.body.grade) || req.body.grade < 0 || req.body.grade > 100) {
//                 return res.send(`Bad request - grade must be a valid grade number - ${req.body.firstname} , ${req.body.grade}`);
//             }

//             // add the new user
//             data[course_id].students[student_id] = req.body;

//             writeFile(JSON.stringify(data, null, 2), () => {
//                 res.status(200).send('new student added');
//             });
//         }, true);        
//     },

//     // DELETE STUDENT FROM COURSE
//     delete_student_from_course: function (req, res) {

//         readFile(data => {
//             // get course id
//             const course_id = req.params.id;
//             const student_id = req.params.studentId;

//             // check if course exists
//             if (!data[course_id]) return res.status(404).send('Bad request - course id not found');

//             // check if student id exists
//             if (!student_id) return res.status(400).send('Bad request - student id is required');

//             // check if student id exists in course
//             if (!(student_id in data[course_id].students)) return res.status(404).send(`Bad request - ${student_id} does not exist in course`);

//             // delete the user
//             delete data[course_id].students[student_id];

//             writeFile(JSON.stringify(data, null, 2), () => {
//                 res.status(200).send(`users id:${student_id} removed`);
//             });
//         }, true);
//     }
};