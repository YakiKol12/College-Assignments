$(function(){
    display_courses();

    $('#closeViewStudentPopup').click(function() {
        $('#popupContainer').fadeOut();
    });

    $('#closeAddStudentPopup').click(function() {
        //close the popup
        $('#addStudentPopupContainer').fadeOut();
    });

    $('#closeEditCoursePopup').click(function() {
        //close the popup
        $('#editCoursePopupContainer').fadeOut();
    });

    $('#closeAddStudentPopup').click(function() {
        //close the popup
        $('#addStudentPopupContainer').fadeOut();
    });

    //method to update the course
    $('#editCourseForm').submit(function(e){
        e.preventDefault();
        let course_id = $(this).attr('course_id');
        
        let course_name = $('#courseName').val();
        let course_lecturer = $('#courseLecturer').val();
        let course_start_date = $('#courseStartDate').val();
        let course_end_date = $('#courseEndDate').val();
        

        $.ajax({
            url: '/courses/' + course_id,
            type: 'PUT',
            data: {
                name: course_name,
                lecturer: course_lecturer,
                start_date: course_start_date,
                end_date: course_end_date,
            },
            success: function(data){
                $('#editCoursePopupContainer').fadeOut();
                location.reload();
            },
            error: function(error){
                console.log(error);
            }
        });
    });

    //method to add a student to the course
    $('#add_student_form').submit(function(e){
        e.preventDefault();
        let course_id = $(this).attr('course_id');

        // get the student information
        let student = $('#student_selection').val();
        let grade = $('#student_grade').val();


        $.ajax({
            url: '/courses/' + course_id,
            type: 'POST',
            data: {
                student_id: student,
                grade: grade
            },
            success: function(data){
                $('#addStudentPopupContainer').fadeOut();
                location.reload();
            },
            error: function(error){
                console.log(error);
            }
        });
    });

// display courses
function display_courses(){
    let courses = {};
    // get the courses information
    $.ajax({
        url: '/courses',
        type: 'GET',
        dataType: 'json',
        success: function(data){
            courses = data;
        },
        error: function(error){
            console.log(error);
        }
    });

    // display the courses information
    setTimeout(function(){
        let courses_table = $('#courses_table');
        Object.keys(courses).forEach((key) => {
            let course = courses[key];
            let row = $('<tr></tr>');
            row.append($('<td></td>').text(course['course_id']));
            row.append($('<td></td>').text(course['name']));
            row.append($('<td></td>').text(course['lecturer']));
            row.append($('<td></td>').text(course['start_date'].split('T')[0]));
            row.append($('<td></td>').text(course['end_date'].split('T')[0]));
            let students_button = $('<button></button>').text('view students');
            students_button.addClass('view_students_button');
            students_button.attr('course_id', course['course_id']);

            let add_student_button = $('<button></button>').text('add student');
            add_student_button.addClass('add_student_button');
            add_student_button.attr('course_id', course['course_id']);
            
            row.append($('<td></td>').append(students_button, add_student_button));

            let delete_button = $('<button>Delete</button>');
            delete_button.addClass('delete_button');
            delete_button.addClass('delete_course_button');
            delete_button.attr('course_id', course['course_id']);
            row.append($('<td></td>').append(delete_button));

            let edit_button = $('<button>Edit</button>');
            edit_button.addClass('update_button');
            edit_button.addClass('update_course_button');
            edit_button.attr('course_id', course['course_id']);
            row.append($('<td></td>').append(edit_button));

            courses_table.append(row);
        });
    }, 100);
}

// delete course
$(document).on('click', '.delete_course_button', function(){
    let course_id = $(this).attr('course_id');
    $.ajax({
        url: '/courses/' + course_id,
        type: 'DELETE',
        success: function(data){
            location.reload();
        },
        error: function(error){
            console.log(error);
        }
    });
});

// update course
$(document).on('click', '.update_course_button', function(){
    let course_id = $(this).attr('course_id');
    //clean the form
    $('#updateCourseForm input').val('');

    // get the course id
    $('#editCourseForm').attr('course_id', course_id);
    $('#editCoursePopupContainer').fadeIn();
});

// view students
$(document).on('click', '.view_students_button', function(){
    let course_id = $(this).attr('course_id');

    let course = {};
    $.ajax({
        url: '/courses/' + course_id,
        type: 'GET',
        dataType: 'json',
        success: function(data){
            course = data;
        },
        error: function(error){
            console.log(error);
        }
    }).then(function(){
        let students_objectIds = course.grades.map((grade) => { return grade.student; });
        
        let students = {};
        // get all the students
        $.ajax({
            url: '/students',
            type: 'GET',
            dataType: 'json',
            success: function(data){
                students = data;
            }
        }).then(function(){

            // create array of students that are in students_objectIds
            let students_in_course = [];
            students.forEach((student) => {
                if(students_objectIds.includes(student._id)){
                    students_in_course.push(student);
                }
                
            });

            // display the students
            let students_table = $('#students_table');
            students_table.empty();
            let row_header = $('<tr></tr>');
            row_header.append($('<th></th>').text('ID'));
            row_header.append($('<th></th>').text('First Name'));
            row_header.append($('<th></th>').text('Last Name'));
            row_header.append($('<th></th>').text('Grade'));
            row_header.append($('<th></th>').text('Picture'));
            row_header.append($('<th></th>').text(''));
            students_table.append(row_header);
            
            students_in_course.forEach((student) => {
                let row = $('<tr></tr>');
                row.append($('<td></td>').text(student.student_id));
                row.append($('<td></td>').text(student.firstname));
                row.append($('<td></td>').text(student.surname));
                row.append($('<td></td>').text(course.grades[students_objectIds.indexOf(student._id)].grade));
                row.append($('<td></td>').append($('<img>').attr('src', student.picture)));
                let delete_button = $('<button>Delete</button>');
                delete_button.addClass('delete_button');
                delete_button.addClass('delete_student_button');
                delete_button.attr('course_id', course_id);
                delete_button.attr('student_id', student._id);
                row.append($('<td></td>').append(delete_button));
                students_table.append(row);
            }
            );
            $('#viewStudentsPopupContainer').fadeIn();

        });

    });
    

    // setTimeout(function(){
    //     let students = course['students'];
        
    //     let students_table = $('#students_table');
    //     students_table.empty();
    //     let row_header = $('<tr></tr>');
    //     row_header.append($('<th></th>').text('ID'));
    //     row_header.append($('<th></th>').text('First Name'));
    //     row_header.append($('<th></th>').text('Last Name'));
    //     row_header.append($('<th></th>').text('Grade'));
    //     row_header.append($('<th></th>').text('Picture'));
    //     row_header.append($('<th></th>').text(''));
    //     students_table.append(row_header);

    //     Object.keys(students).forEach((key) => {
    //         let student = students[key];
    //         let row = $('<tr></tr>');
    //         row.append($('<td></td>').text(student['id']));
    //         row.append($('<td></td>').text(student['firstname']));
    //         row.append($('<td></td>').text(student['surname']));
    //         row.append($('<td></td>').text(student['grade']));
    //         row.append($('<img></img>').attr('src', student['picture']));
            
    //         let delete_button = $('<button>Delete</button>');
    //         delete_button.addClass('delete_button');
    //         delete_button.addClass('delete_student_button');
    //         delete_button.attr('course_id', course_id);
    //         row.append($('<td></td>').append(delete_button));
    //         students_table.append(row);
    //     });
    // }, 100);
    
    $('#popupContainer').fadeIn();
});
  
// delete student
$(document).on('click', '.delete_student_button', function(){
    let course_id = $(this).attr('course_id');
    let student_id = $(this).attr('student_id');

    $.ajax({
        url: '/courses/' + course_id + '/' + student_id,
        type: 'DELETE',
        success: function(data){
            location.reload();
        },
        error: function(error){
            console.log(error);
        }
    });
});

// add student
$(document).on('click', '.add_student_button', function() {
    // get the course id
    let course_id = $(this).attr('course_id');


    //clean the form
    $('#add_student_form input').val('');

    //load all students to the select
    let students_collection = {};
    $.ajax({
        url: '/students',
        type: 'GET',
        dataType: 'json',
        success: function(data){
            students_collection = data;
        },
        error: function(error){
            console.log(error);
        }
    }).then(function(){

        // get the student ids from the course
        let students_objectIds = []; // array of student ids in the course already 
        $.ajax({
            url: '/courses/' + course_id,
            type: 'GET',
            dataType: 'json',
            success: function(data){
                students_objectIds = data.grades.map((grade) => { return grade.student; });
            },
            error: function(error){
                console.log(error);
            }
        }).then(function(){

            // create array of students that are not in students_objectIds
            let students_not_in_course = [];
            students_collection.forEach((student) => {
                if(!students_objectIds.includes(student._id)){
                    students_not_in_course.push(student);
                }
            });

            // add the students to the select
            let select = $('#student_selection');
            select.empty();
            students_not_in_course.forEach((student) => {
                let option = $('<option></option>');
                option.attr('value', student._id);
                option.text(student.firstname + ' ' + student.surname);
                select.append(option);
            });

        });

        //show the form
        $('#addStudentPopupContainer').fadeIn();
        $('#add_student_form').attr('course_id', course_id);
    });
});


});
