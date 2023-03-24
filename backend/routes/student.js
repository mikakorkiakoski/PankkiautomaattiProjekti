const express = require('express');
const router = express.Router();
const student = require('../models/student_model');

//tänne metodeja
router.get('/', function(request, response) {
    student.getAll(function(err, dbResult) {
        if (err) {
            response.json(err);
        } else {
            response.json(dbResult);
        }
    })
});

router.get('/:id',
    function (request, response) {
        student.getById(request.params.id, function (err, dbResult) {
            if (err) {
                response.json(err);
            } else {
                response.json(dbResult[0]); //<-- tulee pelkkä objekti, ei taulukkoa
            }
        })
    });


router.post('/', 
function(request, response) {
  student.add(request.body, function(err, dbResult) {
    if (err) {
      response.json(err);
    } else {
      //response.json(dbResult);
      response.json(dbResult.affectedRows);
    }
  });
});


router.delete('/:id', 
function(request, response) {
  student.delete(request.params.id, function(err, dbResult) {
    if (err) {
      response.json(err);
    } else {
      response.json(dbResult.affectedRows + " affected rows!")
    }
  });
});


router.put('/:id', 
function(request, response) {
  student.update(request.params.id, request.body, function(err, dbResult) {
    if (err) {
      response.json(err);
    } else {
      response.json(dbResult);
    }
  });
});

module.exports = router;