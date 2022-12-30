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
    // post aleatoric number to Pure Data
    var xhr = new XMLHttpRequest();
    // get the local host address
    var host = window.location.hostname;
    // get the port number
    var port = window.location.port;
    // set the url
    var url = 'http://' + host + ':' + port + '/send2pd';
    // open the connection
    xhr.open('POST', url, true);
    // set the content type
    xhr.setRequestHeader('Content-Type', 'application/json');
    // send the request
    var random_number = Math.round(Math.random() * 100);

    // make list of random numbers
    var list = [];
    for (var i = 0; i < 10; i++) {
        list.push(Math.round(Math.random() * 100));
    }
    xhr.send(JSON.stringify({ 'number': random_number}));
}

// =======================================
function sendMessage() {
    // post aleatoric number to Pure Data
    var xhr = new XMLHttpRequest();
    // get the local host address
    var host = window.location.hostname;
    // get the port number
    var port = window.location.port;
    // set the url
    var url = 'http://' + host + ':' + port + '/send2pd';
    // open the connection
    xhr.open('POST', url, true);
    // set the content type
    xhr.setRequestHeader('Content-Type', 'application/json');
    // send the request
    var message = document.getElementById('message').value;
    // make list of random numbers
    xhr.send(JSON.stringify({ 'symbol': message}));
}

// =======================================
function sendList() {
    // post aleatoric number to Pure Data
    var xhr = new XMLHttpRequest();
    // get the local host address
    var host = window.location.hostname;
    // get the port number
    var port = window.location.port;
    // set the url
    var url = 'http://' + host + ':' + port + '/send2pd';
    // open the connection
    xhr.open('POST', url, true);
    // set the content type
    xhr.setRequestHeader('Content-Type', 'application/json');
    // send the request
    var list = [];
    for (var i = 0; i < 10; i++) {
        list.push(Math.round(Math.random() * 100));
    }
    xhr.send(JSON.stringify({ 'list': list}));
}


// =======================================
function updateimage() {
    var image = document.getElementsByTagName('img')[0];
    image.src = 'score' + '.png' + '?' + new Date().getTime(); // prevent caching
    updateprogress(8000);
}

// =======================================
updateimage();