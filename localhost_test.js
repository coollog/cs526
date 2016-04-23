var sys = require('sys');
var exec = require('child_process').exec;
var querystring = require('querystring');
var http = require('http');

function ChainRepl() {
  var chain = [
    2234,
    2235,
    2236,
    2237,
    2238,
    2239,
    2240,
    2241
  ];

  (function runChain(chain) {
    exec('killall -9 cs426_graph_server');

    for (var i = chain.length - 1; i >= 0; i --) {
      var port = chain[i];

      var nextNode = '';
      if (i < chain.length - 1) {
        var nextPort = chain[i + 1];
        nextNode = ' -b localhost:' + nextPort + '/rpc';
      }

      var cmd = './cs426_graph_server ' + port + nextNode;

      console.log('Running cmd: ' + cmd);
      exec(cmd);
    }
  })(chain);

  function curl(port, route, data, callback) {
    data = JSON.stringify(data);

    var options = {
      host: 'localhost',
      port: port,
      path: '/api/v1/' + route,
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(data)
      }
    }

    var req = http.request(options, function (res) {
      if (res.statusCode == 400) {
        callback(400, '{}');
        return;
      }

      res.setEncoding('utf8');
      res.on('data', function (json) {
        callback(res.statusCode, json);
      });
    });

    req.write(data);
    req.end();
  }

  function addNode(id, callback) {
    var port = chain[0];

    var data = {
      node_id: id
    };

    curl(port, 'add_node', data, callback);
  }

  function addEdge(id1, id2, callback) {
    var port = chain[0];

    var data = {
      node_a_id: id1,
      node_b_id: id2
    };

    curl(port, 'add_edge', data, callback);
  }

  function removeNode(id, callback) {
    var port = chain[0];

    var data = {
      node_id: id
    };

    curl(port, 'remove_node', data, callback);
  }

  function removeEdge(id1, id2, callback) {
    var port = chain[0];

    var data = {
      node_a_id: id1,
      node_b_id: id2
    };

    curl(port, 'remove_edge', data, callback);
  }

  function check(route, data, callback) {
    var correct = true;
    var count = 0;

    for (var i = 0; i < chain.length; i ++) {
      var port = chain[i];

      (function sendCurl(port) {
        curl(port, route, data, function (code, json) {
          count ++;

          var json = JSON.parse(json);

          if (json.in_graph == 1) {
            console.log('Checked ' + port + ' exists!');
          } else {
            console.log('Checked ' + port + ' doesn\'t exist!');
            correct = false;
          }

          if (count == chain.length) {
            callback(correct);
          }
        });
      })(port);
    }
  }

  function checkNode(id, callback) {
    console.log('Checking for node ' + id);

    var data = {
      node_id: id
    };

    check('get_node', data, callback);
  }

  function checkEdge(id1, id2, callback) {
    console.log('Checking for edge ' + id1 + '-' + id2);

    var data = {
      node_a_id: id1,
      node_b_id: id2
    };

    check('get_edge', data, callback);
  }

  this.addNode = addNode;
  this.addEdge = addEdge;
  this.removeNode = removeNode;
  this.removeEdge = removeEdge;

  this.checkNode = checkNode;
  this.checkEdge = checkEdge;
}

var chain = new ChainRepl();

chain.addNode(1, function (code, json) {
  if (code != 200) { console.log('Got code: ' + code); return; }

  console.log('Node 1 added!');

  chain.checkNode(1, function (correct) {
    if (!correct) { console.log('Incorrect!'); return; }

    chain.addNode(2, function (code, json) {
      if (code != 200) { console.log('Got code: ' + code); return; }

      console.log('Node 2 added!');

      chain.checkNode(2, function (correct) {
        if (!correct) { console.log('Incorrect!'); return; }

        chain.addEdge(1, 2, function (code, json) {
          if (code != 200) { console.log('Got code: ' + code); return; }

          console.log('Edge 1-2 added!');

          chain.checkEdge(1, 2, function (correct) {
            if (!correct) { console.log('Incorrect!'); return; }

            chain.removeNode(2, function (code, json) {
              if (code != 200) { console.log('Got code: ' + code); return; }

              console.log('Node 2 removed!');

              chain.checkEdge(1, 2, function (correct) {
                if (correct) { console.log('Edge still exists!'); return; }

                chain.addNode(2, function (code, json) {
                  if (code != 200) { console.log('Got code: ' + code); return; }

                  console.log('Node 2 added!');

                  chain.addEdge(1, 2, function (code, json) {
                    if (code != 200) { console.log('Got code: ' + code); return; }

                    console.log('Edge 1-2 added!');

                    chain.removeEdge(2, 1, function (code, json) {
                      if (code != 200) { console.log('Got code: ' + code); return; }

                      chain.checkEdge(1, 2, function (correct) {
                        if (correct) { console.log('Edge still exists!'); return; }

                        console.log("ALL TESTS PASSED!");
                      });
                    });
                  });
                });
              });
            });
          });
        });
      });
    })
  });
});