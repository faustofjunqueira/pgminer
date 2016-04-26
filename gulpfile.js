'use strict'
/**
 * Created by fausto on 4/21/16.
 */

var gulp = require('gulp'),
  jshint = require('gulp-jshint'),
  concat = require('gulp-concat'),
  uglify = require('gulp-uglify'),
  es = require('event-stream'),
  htmlmin = require('gulp-htmlmin'),
  cleanCSS = require('gulp-clean-css'),
  runSequence = require('run-sequence'),
  rename = require('gulp-rename'),
  sass = require('gulp-sass'),
  sourcemaps = require('gulp-sourcemaps'),
  header = require('gulp-header'),
  fileinclude = require('gulp-file-include');

var name = 'pgminer';
var base = './src';
var script = base + '/js/**.js';
var styles = base + '/sass/**.scss';
var htmlBase = base + '/html';
var htmls = htmlBase + '/*.html';
var htmlPartial = htmlBase + '/partial';


var destJs = './js';
var destCss = './css';
var destHtml = './';
var destFonts = './fonts';

var headertext = '\n/*!\nDate:'+ (new Date).toString() +'\nPgminer - Postgres Extension\n */\n';

gulp.task('jshint', function () {
  return gulp.src(script)
    .pipe(jshint())
    .pipe(jshint.reporter('default'));
});

gulp.task('uglify', function () {
  return es.merge([
      gulp.src([
        "./node_modules/jquery/dist/jquery.min.js",
        "./node_modules/bootstrap/dist/js/bootstrap.min.js"
      ]),
      gulp.src('./src/**/*.js')
        .pipe(sourcemaps.init())
        .pipe(concat('scripts.js'))
        .pipe(uglify())
        .pipe(sourcemaps.write('./maps'))
        .pipe(header(headertext))
    ])
    .pipe(concat(name+'.min.js'))
    .pipe(gulp.dest(destJs));
});

gulp.task('htmlmin', function () {
  return gulp.src(htmls)
    .pipe(fileinclude({
      prefix: '@@',
      basepath: htmlPartial
    }))
    .pipe(htmlmin({collapseWhitespace: true}))
    .pipe(gulp.dest(destHtml))
});

gulp.task('css', function () {
  return es.merge([
      gulp.src([
        "./node_modules/bootstrap/dist/css/bootstrap.min.css",
        "./node_modules/bootstrap/dist/css/bootstrap-theme.min.css"
      ]),
      gulp.src(styles).pipe(concat('sass.scss')).pipe(sass({outputStyle: 'compressed'}).on('error', sass.logError))
    ])
    .pipe(cleanCSS())
    .pipe(concat(name+'.min.css'))
    .pipe(gulp.dest(destCss));
});

gulp.task('fonts', function(){
  return gulp.src(['node_modules/bootstrap/dist/fonts/*'])
    .pipe(gulp.dest(destFonts));
});

gulp.task('dist', function(){
  console.log('Watching...');
  gulp.watch(script, ['jshint', 'uglify']);
  gulp.watch(styles, ['css']);
  gulp.watch(htmls, ['htmlmin']);
});

gulp.task('default',function(cb){
  return runSequence(['jshint', 'uglify', 'htmlmin', 'css', 'fonts'], 'dist', cb);
});