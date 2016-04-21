/**
 * Created by fausto on 4/21/16.
 */

var gulp = require('gulp');
var less = require('gulp-less');


gulp.task('less', function(){
  return gulp.src('less/agency.less')
    .pipe(less({
      paths: ['less/mixins.less', 'less/variables.less']
    }))
    .pipe(gulp.dest('./css'));
});

gulp.task('default',function(){

  gulp.watch('less/*', ['less']);
});