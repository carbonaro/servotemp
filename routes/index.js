var os = require('os');

/*
 * GET home page.
 */

exports.index = function(req, res){
  res.render('index', { title: 'Servotemp', host: getHostAddress(), port: process.env.PORT || 8088});
};

exports.getByDate = function(req, res) {
  var min = req.param('min') || "-inf";
  var max = req.param('max') || "+inf";
  var data = [];
  db.zrevrangebyscore('servotemp:measures', max, min, function(err, results) {
    data = results.map(function(e,i) {
      try {
        return JSON.parse(e);
      } catch (ex) {
        return null;
      }
    });
    data = data.filter(function(e) { return (e !== null) });
    res.set('Content-Type', 'application/json');
    res.send(data);
  });
}

function getHostAddress() {
  var ifaces = os.networkInterfaces();
  var address = '127.0.0.1';
  var iface= process.env.IFACE || 'wlan0';
  if ('undefined' != typeof(ifaces[iface])) {
    ifaces[iface].forEach(function(details) {
      if (details.family=='IPv4')
        address = details.address;
    });
  }
  return address;
}