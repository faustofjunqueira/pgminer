var express = require('express');
var app = express();
app.use('/pgminer',express.static('./'));
app.listen(3000);