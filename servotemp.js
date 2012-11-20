var sio = require('socket.io')
  , serial = require('serialport')
  , child = require('child_process')
  , moment = require('moment')
  , fs = require('fs')
    redis = require("redis");

var express = require('express')
  , routes = require('./routes')
  , http = require('http')
  , path = require('path');

var app = express();

function connectToArduino(callback) {
  // Seems to work on a mac and on a Raspberry Pi
  child.exec('ls /dev | grep -E "tty\.usb|ttyACM0"', function(err, stdout, stderr){
    var possible = stdout.slice(0, -1).split('\n'),
        found = false;
    for (var i in possible) {
      var tempSerial, err;
      try {
        tempSerial = new serial.SerialPort('/dev/' + possible[i], {
          baudrate: 9600,
          parser: serial.parsers.readline("\n")
        });
      } catch (e) {
        console.log(e);
        err = e;
      }
      if (!err) {
        found = tempSerial;
        console.log('found board at /dev/' + possible[i]);
        break;
      }
    }
    if (found) callback(null, tempSerial);
    else callback(new Error('Could not find Arduino'));
  });
}

function parseSerialData(data) {
  // Ex: @085,  17.00,  20.50
  var line, fd, elms, ts, value = {};
  data = data.replace('\\r', '').replace(/\"/g,''); // Hack to filter unescaped \r from Redis publications
  var match = data.match(/^@[0-9]{3},[-| ].{6},[-| ].{6}\!/g);
  if (match) {
    ts = moment().format();
    line = ts + "," + data.replace("@","").replace("!","");
    elms = line.split(",");
    value = {date: ts, angle: elms[1], ambiant: elms[2], object: elms[3]};
    fd = fs.openSync(path.join(process.cwd(), 'public','temperatures.csv'), 'a');
    fs.writeSync(fd, line + "\n");
    fs.closeSync(fd);
    console.log("csv:", line);
    db.zadd('servotemp:measures', new Date(ts).getTime(), JSON.stringify(value), function(err, res) {
      if (err)
        console.log('error saving to Redis');
      else
        console.log("redis: stored measure");
    });
    io.sockets.emit('data', value);
  }
}

app.configure(function(){
  app.set('port', process.env.PORT || 8088);
  app.set('views', __dirname + '/views');
  app.set('view engine', 'jade');
  app.use(express.favicon());
  app.use(express.logger('dev'));
  app.use(express.bodyParser());
  app.use(express.methodOverride());
  app.use(app.router);
  app.use(express.static(path.join(__dirname, 'public')));
});

app.configure('development', function(){
  app.use(express.errorHandler());
});

if (process.env.SLAVE) {
  var sub = redis.createClient();
  sub.subscribe('servotemp');
  sub.on('message', function(channel, data) {
    if (channel == 'servotemp')
      parseSerialData(data);
  });
} else {
  var pub = redis.createClient();
  connectToArduino(function(err, serialPort) {
    if (err) {
      console.log(err);
    } else {
      serialPort.on('data', function(data) {
        parseSerialData(data);
        if ('undefined' != typeof(pub)) {
          pub.publish('servotemp', JSON.stringify(data));
          console.log("redis pub:", data);
        }
      });
    }
  });
}

var db = global.db = redis.createClient();

app.get('/', routes.index);
app.get('/measures', routes.getByDate);

var server = http.createServer(app);
var io = sio.listen(server);
server.listen(app.get('port'), function(){
  console.log("Express server listening on port " + app.get('port'));
});


io.sockets.on('connection', function (socket) {
  console.log('got a new socket');
});