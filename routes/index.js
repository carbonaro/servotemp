var os = require('os');
/*
 * GET home page.
 */

exports.index = function(req, res){
  res.render('index', { title: 'Servotemp', host: getHostAddress(), port: process.env.PORT || 8088});
};

function getHostAddress() {
  var ifaces=os.networkInterfaces();
  var address = '127.0.0.1';
  var iface= process.env.IFACE || 'eth0';
  if ('undefined' != typeof(ifaces[iface])) {
    ifaces[iface].forEach(function(details) {
      if (details.family=='IPv4')
        address = details.address;
    });
  }
  return address;
}