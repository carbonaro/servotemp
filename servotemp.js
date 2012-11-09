var sio = require('socket.io')
  , serial = require('serialport')
  , express = require('express')
  , child = require('child_process')
  , moment = require('moment');

var app = express();

app.get('/', function(req, res){
  res.send('Hello World');
});

app.listen(8088);

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
  var line;
  var match = data.match(/^@[0-9]{3},[-| ].{6},[-| ].{6}\!/g);
  if (match) {
    line = moment().format() + "," + data.replace("@","").replace("!","");
    console.log(line);
  }
}

connectToArduino(function(err, serialPort) {
  if (err) {
    console.log(err);
  } else {
    serialPort.on('data', parseSerialData);
  }
});