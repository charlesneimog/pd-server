// =======================================
// load javascript file
// =======================================

function updateprogress(duration) {
    var start = new Date().getTime();
    var interval = setInterval(function() {
        var now = new Date().getTime();
        var progress = now - start;
        var div = document.getElementsByTagName('div')[0];
        var div_width = Math.min(progress / duration * 100, 100);
        div.style.width =  div_width + '%';
        if (div_width == 100) {
            clearInterval(interval);
            div.style.width = 100;
        }              
    }, 30); 
}

// =======================================
function updateimage() {
    var image = document.getElementsByTagName('img')[0];
    image.src = 'score' + '.png' + '?' + new Date().getTime();
    // read json file inside flute/update_rate.json and get the update rate
    updateprogress(8000);
}
            
updateimage();