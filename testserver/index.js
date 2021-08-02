const express = require('express')
const app = express()
//const router = express.Router();

app.use(express.static('../data', {fallthrough: true}));

const config = {
  itIs: 1,
  rainbow: 1,
  rainbowSpeed:1,
  tmpl: 1,
  colors: "255,128,128,40,140,40,56,56,156",
  fg: "#ff5555",
  bg: "#55ff55",
  s: "#5555ff",
  mode: 1,
  timezone: 2,
  heartbeat: false,
  ntpserver: "129.3.2.2",
  autoOnOff: false,
  autoOn: "06:00",
  autoOff: "23:45",
  brightness: 240
};

app.get('/:cmd', function (req, res) {
  let what = req.params.cmd.substr(3);
  if( req.params.cmd == 'config') {
    res.status(200).json(config);
  } else if( req.params.cmd.startsWith('get') ) {
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