

function timeUpdate() {
    var a = $(".progress"),
        d = 0;
    "Infinity" == c.duration ? d = 100 : c.currentTime > 0 && (d = Math.floor(100 / c.duration * c.currentTime)), a.stop().animate({
        width: d + "%"
    }, 500), $("#music-player #time").html(b(c.currentTime)), $("#music-player #total-time").html(b(c.duration));

    localStorage.setItem("CurrTime", c.currentTime);
}

function b(a) {
    return minutes = Math.floor(a / 60), seconds = Math.floor(a % 60), seconds = seconds >= 10 ? seconds : ("0" + seconds).slice(-2), minutes + ":" + seconds
}

function bb(a) {
    return minutes = Math.floor(a / 60), minutes = minutes >= 10 ? minutes : "" + minutes, seconds = Math.floor(a % 60), seconds = seconds >= 10 ? seconds : ("0" + seconds).slice(-2), minutes + "." + seconds
}


if (document.getElementById("audio3") != null) {
    var c = document.getElementById("audio3");

    c.currentTime = localStorage.getItem("CurrTime");

    var homeAudio = document.getElementById("audios");
    $(document).on("click", ".play", function () {
        // 
        if (homeAudio != null && homeAudio.played) {
            homeAudio.pause();
            document.getElementById("startIcon").style.display = "none";
            document.getElementById("stopIcon").style.display = "block";
        }
        c.play(), $(".play").hide(), $(".pause").show()
    }), $(document).on("click", ".pause", function () {
        c.pause(), $(".pause").hide(), $(".play").show()
    }), c.addEventListener("timeupdate", timeUpdate);
    volumeslider = document.getElementById("volumeslider1"),
    mutebtn = document.getElementById("mutebtn1");
   
}


$('#mutebtn1').off('click').click(function mute() {
    var vid = document.getElementById("audio3");
    if (vid.muted) {
        vid.muted = false;
        volumeslider.value = 100;
        mutebtn1.src = "/assets/img/vol.png";
    } else {
        vid.muted = true;
        volumeslider.value = 0;
        mutebtn1.src = "/assets/img/muteSong.png";
    }
});


$('#volumeslider1').off('click').click(function setvolume1() {
    var vid = document.getElementById("audio3");
    vid.volume = volumeslider.value / 100;
});


$(".bar-bg").off('click').click(function (e) {

    var $this = $(this);
    // to get part of width of progress bar clicked
    var widthclicked = e.pageX - $this.offset().left;
    var totalWidth = $this.width(); // can also be cached somewhere in the app if it doesn't change

    // do calculation of the seconds clicked
    var calc = (widthclicked / totalWidth * c.duration); // get the percent of bar clicked and multiply in by the duration
    var calc_fix = calc.toFixed(00);
    c.currentTime = calc_fix;

});


