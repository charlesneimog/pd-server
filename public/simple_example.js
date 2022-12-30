// =======================================
// load javascript file
// =======================================
function updateprogress(duration) {
    var start = new Date().getTime();
    var interval = setInterval(function() {
        var now = new Date().getTime();
        var progress = now - start;
        // get ScoreTime id
        var scoretime = document.getElementById('ScoreTime');
        var div_width = Math.min(progress / duration * 100, 100);
        scoretime.style.width =  div_width + '%';
        if (div_width == 100) {
            clearInterval(interval);
            scoretime.style.width = 100;
        }              
    }, 30); 
}

// =======================================
function sendNumber() {
    var xhr = new XMLHttpRequest();
    var host = window.location.hostname;
    var port = window.location.port;
    var protocol = window.location.protocol;
    var url = protocol + '//' + host + ':' + port + '/send2pd'; // WARNING: This is an standard, all the requests must be sent to this url
    xhr.open('POST', url, true);
    xhr.setRequestHeader('Content-Type', 'application/json');
    var random_number = Math.round(Math.random() * 100);
    xhr.send(JSON.stringify({ 'number': random_number}));
}

// =======================================
function sendMessage() {
    var xhr = new XMLHttpRequest();
    var host = window.location.hostname;
    var port = window.location.port;
    var protocol = window.location.protocol;
    var url = protocol + '//' + host + ':' + port + '/send2pd'; // WARNING: This is an standard, all the requests must be sent to this url
    xhr.open('POST', url, true);
    xhr.setRequestHeader('Content-Type', 'application/json');
    var message = document.getElementById('message').value;
    xhr.send(JSON.stringify({ 'symbol': message}));
}

// =======================================
function sendList() {
    var xhr = new XMLHttpRequest();
    var host = window.location.hostname;
    var port = window.location.port;
    var protocol = window.location.protocol;
    var url = protocol + '//' + host + ':' + port + '/send2pd'; // WARNING: This is an standard, all the requests must be sent to this url
    xhr.open('POST', url, true);
    xhr.setRequestHeader('Content-Type', 'application/json');
    var list = [];
    for (var i = 0; i < 100; i++) {
        list.push(Math.round(Math.random() * 100));
    }
    xhr.send(JSON.stringify({ 'list': list}));
}

// =======================================
// TODO: implement websocket

// function startWebsocket() {
//     // get the local host address
//     var host = window.location.hostname;
//     // get the port number
//     var port = window.location.port;
//     // set the url
//     var url = 'ws://' + host + ':' + port + '/ws';
//     // open the connection
//     var ws = new WebSocket(url);
//     // set the onmessage function
//     ws.onmessage = function (event) {
//         var data = JSON.parse(event.data);
//         var score = data.score;
//         var scoretime = document.getElementById('ScoreTime');
//         scoretime.innerHTML = score;
//     };
// }

// =======================================
function updateimage() {
    var image = document.getElementsByTagName('img')[0];
    image.src = 'score' + '.png' + '?' + new Date().getTime(); // prevent caching
    updateprogress(8000);
}

// =======================================
updateimage();