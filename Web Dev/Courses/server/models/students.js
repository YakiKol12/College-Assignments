const mongoose = require('mongoose')
const validator = require('validator')

var StudentSchema = new mongoose.Schema({
    student_id: {
        type: String,
        required: true,
        unique: true,
        trim: true
    },
    firstname: {
        type: String,
        required: true,
        trim: true
    },
    surname: {
        type: String,
        required: true,
        trim: true
    },
    picture: {
        type: String,
        required: true,
        trim: true
    },
}, { timestamps: true }
);

const Student = mongoose.model('Students', StudentSchema);

module.exports = Student;