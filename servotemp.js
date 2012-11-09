var sio = require('socket.io')
  , Serial = require('serialport')
  , express = require('express')
  , child = require('child_process');

const SERIAL_START_CHAR = '$';
const SERIAL_END_CHAR = '\r';

function connectToArduino(callback) {
  // Seems to work on a mac and on a Raspberry Pi
  child.exec('ls /dev | grep -E "tty\.usb|ttyACM0"', function(err, stdout, stderr){
    var possible = stdout.slice(0, -1).split('\n'),
        found = false;
    for (var i in possible) {
      var tempSerial, err;
      try {
        tempSerial = new serial.SerialPort('/dev/' + possible[i], {
          baudrate: 115200,
          parser: serial.parsers.readline("\n")
        });
      } catch (e) {
        err = e;
      }
      if (!err) {
        found = tempSerial;
        self.log('info', 'found board at ' + tempSerial.port);
        break;
      }
    }
    if (found) cb(null, found);
    else cb(new Error('Could not find Arduino'));
  });
}
