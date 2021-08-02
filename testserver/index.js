const express = require('express')
const app = express()
//const router = express.Router();

app.use(express.static('../data', {fallthrough: true}));

const config = {
  itIs: 1,
  rainbow: 1,
  rainbowSpeed:1,
  template: 0,
  colors: "255,128,128,40,140,40,56,56,156",
  mode: "1",
  timezone: "2",
  heartbeat: 0,
  ntpserver: "129.3.2.2",
  autoOnOff: 0,
  autoOn: "06:00",
  autoOff: "23:45"
};

app.get('/:cmd', function (req, res) {
  let what = req.params.cmd.substr(3);
  if( req.params.cmd.startsWith('get') ) {
    if( what === 'var') {
      what = req.query.name;
      console.log("request for name: "+what+ " -> "+config[what]);
      res.status(200).send(config[what].toString());
    } else {
      console.log("request for name: "+what+ " -> "+config[what]);
      res.status(200).send(config[what].toString());
    }
  } else if(req.params.cmd.startsWith('set')) {
    res.status(200).send('OK');
  }
})

console.log("server is listening on 3000");
app.listen(3000)