const mongoose = require('mongoose')
const validator = require('validator')

var CourseSchema = new mongoose.Schema({
    course_id: {
        type: String,
        required: true,
        trim: true,
        unique: true
    },
    name: {
        type: String,
        required: true,
        trim: true
    },
    lecturer: {
        type: String,
        required: true,
        trim: true
    },
    start_date: {
        type: Date,
        required: true,
        trim: true
    },
    end_date: {
        type: Date,
        required: true,
        trim: true
    },
    grades: {
        type: [{
            student: {
                type: mongoose.Schema.Types.ObjectId,
                ref: 'students', // Reference to the Student model/collection
                required: true,
                trim: true
            },
            grade: {
                type: Number,
                required: true,
                trim: true
            },
        }],
        default: []
    },
}, { timestamps: true }
);

const Course = mongoose.model('Courses', CourseSchema);

module.exports = Course;