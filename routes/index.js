var os = require('os');
/*
 * GET home page.
 */

exports.index = function(req, res){
  res.render('index', { title: 'Servotemp', host: getHostAddress(), port: process.env.PORT || 8088});
};

function getHostAddress() {
  var ifaces=os.networkInterfaces();
  var eth0_address = '127.0.0.1';
  if ('undefined' != typeof(ifaces.eth0)) {
    ifaces.eth0.forEach(function(details) {
      if (details.family=='IPv4')
        eth0_address = details.address;
    });
  }
  return eth0_address;
}