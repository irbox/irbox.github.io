"use strict";
// Snackbar utility (existing)
var e = {},
    t = document.querySelector(".app__snackbar"),
    a = null;
e.show = (e, o = 4e3) => {
    e && (a && a.remove(), (a = document.createElement("div")).className = "app__snackbar-msg", a.textContent = e, t.appendChild(a), setTimeout((() => {
        a && a.parentNode === t && a.remove(); // Check if still child before removing
    }), o))
};

// QR Scanner Logic (existing)
var o = {};

function i(e) { // Renamed 's' to 'e' for clarity (isImageMode)
    !e && window.isMediaStreamAPISupported ? o.webcam = document.querySelector("video") : o.webcam = document.querySelector("img")
}
o.active = !1, o.webcam = null, o.canvas = null, o.ctx = null, o.decoder = null, o.setCanvas = () => {
    o.canvas = document.createElement("canvas"), o.ctx = o.canvas.getContext("2d")
}, o.init = () => {
    var t = !1; // Renamed 'e' to 't' for clarity (isCanvasDrawn)

    function a() { // Renamed 's' to 'a' for clarity (setCanvasDimensions)
        o.canvas.width = window.innerWidth, o.canvas.height = window.innerHeight
    }

    function n(e) { // Renamed 'i' to 'n' for clarity (startStream)
        navigator.mediaDevices.getUserMedia(e).then((function(e) {
            o.webcam.srcObject = e, o.webcam.setAttribute("playsinline", !0), o.webcam.setAttribute("controls", !0), setTimeout((() => {
                document.querySelector("video").removeAttribute("controls")
            }))
        })).catch((function(e) {
            console.log("Error occurred ", e), d() // Call noCamera
        }))
    }

    function d() { // Renamed 'r' to 'd' for clarity (noCamera)
        window.noCameraPermission = !0, document.querySelector(".custom-scanner").style.display = "none", e.show("Unable to access the camera", 1e4)
    }
    i(), o.setCanvas(), o.decoder = new Worker("decoder.js"), window.isMediaStreamAPISupported ? o.webcam.addEventListener("play", (function(e) { // 'e' here is event, fine
        t || (a(), t = !0)
    }), !1) : a(), window.isMediaStreamAPISupported && navigator.mediaDevices.enumerateDevices().then((function(e) { // 'e' here is devices, fine
        var t, o = e.filter((function(e) { // 'e' here is device, fine
            if (e.label.split(",")[1], "videoinput" == e.kind) return e
        }));
        o.length > 1 ? (t = { // Renamed 'a' to 'o' for clarity (cameras)
            video: {
                mandatory: {
                    sourceId: o[o.length - 1].deviceId ? o[o.length - 1].deviceId : null
                }
            },
            audio: !1
        }, window.iOS && (t.video.facingMode = "environment"), n(t)) : o.length ? (t = {
            video: {
                mandatory: {
                    sourceId: o[0].deviceId ? o[0].deviceId : null
                }
            },
            audio: !1
        }, window.iOS && (t.video.facingMode = "environment"), t.video.mandatory.sourceId || window.iOS ? n(t) : n({
            video: !0
        })) : n({
            video: !0
        })
    })).catch((function(t) { // 'e' to 't' for error
        d(), console.error("Error occurred : ", t)
    }))
}, o.scan = function(e, t) { // 'e' is callback, 't' is isImageMode
    function a() { // Renamed 's' to 'a' for clarity (tick)
        if (o.active) try {
            o.ctx.drawImage(o.webcam, 0, 0, o.canvas.width, o.canvas.height);
            var e = o.ctx.getImageData(0, 0, o.canvas.width, o.canvas.height); // 'e' is imageData
            e.data && o.decoder.postMessage(e)
        } catch (e) { // 'e' is error
            "NS_ERROR_NOT_AVAILABLE" == e.name && setTimeout(a, 0)
        }
    }
    o.active = !0, o.setCanvas(), o.decoder.onmessage = function(t) { // 't' is message event
        if (t.data.length > 0) {
            var i = t.data[0][2]; // 'i' is qrData
            o.active = !1, e(i) // Call the success callback
        }
        setTimeout(a, 0) // Continue scanning OR prepare for next scan
    }, setTimeout((() => {
        i(t) // Setup webcam/img based on isImageMode
    })), a() // Start the scanning loop
};

// Service Worker (existing)
"serviceWorker" in navigator && window.addEventListener("load", (() => {
    navigator.serviceWorker.register("/service-worker.js").then((t => { // 't' is registration
        localStorage.getItem("offline") || (localStorage.setItem("offline", !0), e.show("App is ready for offline usage.", 5e3))
    })).catch((e => { // 'e' is error
        console.log("SW registration failed: ", e)
    }))
}));

// DOMContentLoaded (Main app logic)
window.addEventListener("DOMContentLoaded", (() => {
    window.iOS = ["iPad", "iPhone", "iPod"].indexOf(navigator.platform) >= 0,
    window.isMediaStreamAPISupported = navigator && navigator.mediaDevices && "enumerateDevices" in navigator.mediaDevices,
    window.noCameraPermission = !1;

    var scannedQrData = null, // Renamed 'e'
        imgElementForFileScan = null, // Renamed 't'
        selectPhotosButton = document.querySelector(".app__select-photos"), // Renamed 'a'
        resultDialog = document.querySelector(".app__dialog"), // Renamed 'i'
        resultDialogOverlay = document.querySelector(".app__dialog-overlay"), // Renamed 'n'
        openLinkButton = document.querySelector(".app__dialog-open"), // Renamed 'd'
        closeDialogButton = document.querySelector(".app__dialog-close"), // Renamed 'r'
        scannerLine = document.querySelector(".custom-scanner"), // Renamed 'c'
        scannerImgFrame = document.querySelector(".app__scanner-img"), // Renamed 'l' - Note: this is hidden by new CSS
        resultInput = document.querySelector("#result"); // Renamed 's'

    document.querySelector(".app__help-text"); // This element is used by new CSS, ensure it exists
    // document.querySelector(".app__header-icon svg"); // SVG inside is styled, element itself is headerIconElement
    document.querySelector("video"); // Used by o.webcam

    var headerIconElement = document.querySelector(".app__header-icon"); // Renamed 'u' - THIS IS NOW THE RELOAD BUTTON

    // Variables for "About" dialog - no longer used for showing/hiding, but keep if other parts of your code might reference them.
    // If not, they can be safely removed. For now, just commenting out their usage.
    // var infoDialog = document.querySelector(".app__infodialog"); // Renamed 'p'
    // var infoDialogCloseButton = document.querySelector(".app__infodialog-close"); // Renamed 'm'
    // var infoDialogOverlay = document.querySelector(".app__infodialog-overlay"); // Renamed 'v'

    function displayScannerUI(isImageMode = !1) { // Renamed 'y', 't' to 'isImageMode'
        if (window.isMediaStreamAPISupported && !window.noCameraPermission) {
            scannerLine.style.display = "block";
            // scannerImgFrame.style.display = "block"; // This SVG frame is hidden by new CSS, scannerLine is sufficient
        }
        if (isImageMode) { // If it's image mode, ensure scanner line is visible
             scannerLine.style.display = "block";
            // scannerImgFrame.style.display = "block";
        }

        o.scan((qrData => { // Renamed 't' to 'qrData'
            scannedQrData = qrData;
            resultInput.value = qrData;
            resultInput.select();
            scannerLine.style.display = "none";
            // scannerImgFrame.style.display = "none";

            // URL Check (existing)
            ((urlToCheck = "") => !(!urlToCheck || "string" != typeof urlToCheck) && new RegExp("^(https?:\\/\\/)?((([a-z\\d]([a-z\\d-]*[a-z\\d])*)\\.)+[a-z]{2,}|((\\d{1,3}\\.){3}\\d{1,3}))(\\:\\d+)?(\\/[-a-z\\d%_.~+]*)*(\\?[;&a-z\\d%_.~+=-]*)?(\\#[-a-z\\d_]*)?$", "i").test(urlToCheck))(qrData) && (openLinkButton.style.display = "inline-flex"); // Use inline-flex from new CSS

            resultDialog.classList.remove("app__dialog--hide");
            resultDialogOverlay.classList.remove("app__dialog--hide");
        }), isImageMode)
    }

    function resetAndPrepareForScan() { // Renamed 'w'
        scannedQrData = null;
        resultInput.value = "";
        if (!window.isMediaStreamAPISupported && imgElementForFileScan) { // If not mediastream, clear the img
            imgElementForFileScan.src = "";
            imgElementForFileScan.className = "";
        }
        openLinkButton.style.display = "none"; // Hide open button initially
        resultDialog.classList.add("app__dialog--hide");
        resultDialogOverlay.classList.add("app__dialog--hide");
        displayScannerUI(); // Restart scanning with camera
    }

    window.appOverlay = document.querySelector(".app__overlay"); // Used for border style, new CSS handles this differently

    window.addEventListener("load", (event => { // 'e' is event
        o.init();
        setTimeout((() => {
            // window.appOverlay.style.borderStyle = "solid"; // New CSS handles overlay visuals
            if (window.isMediaStreamAPISupported) {
                displayScannerUI();
            }
        }), 1e3); // Delay to allow camera init

        // File input setup (existing logic)
        (function() {
            var e = document.createElement("input"); // 'e' is fileInput
            e.setAttribute("type", "file");
            e.setAttribute("capture", "camera"); // This is often for direct camera capture, not just gallery
            e.id = "camera"; // Original ID, might not be strictly needed if not selected elsewhere
            // window.appOverlay.style.borderStyle = ""; // New CSS handles this

            selectPhotosButton.style.display = "flex"; // Ensure FAB is visible

            imgElementForFileScan = document.createElement("img"); // Assign to 't' (now imgElementForFileScan)
            imgElementForFileScan.src = "";
            imgElementForFileScan.id = "frame"; // Original ID

            var appLayoutContent = document.querySelector(".app__layout-content"); // Renamed 'o'
            appLayoutContent.appendChild(e); // Append file input
            appLayoutContent.appendChild(imgElementForFileScan); // Append img element

            selectPhotosButton.addEventListener("click", (() => {
                scannerLine.style.display = "none"; // Hide scanner line when picking file
                // scannerImgFrame.style.display = "none";
                e.click(); // Trigger file input
            }));

            e.addEventListener("change", (event => { // 'e' here is the change event
                if (event.target && event.target.files.length > 0) {
                    imgElementForFileScan.className = "app__overlay"; // This class might conflict or be unneeded with new CSS
                    imgElementForFileScan.src = URL.createObjectURL(event.target.files[0]);

                    if (!window.noCameraPermission) { // If camera permission exists, show scanner line
                        scannerLine.style.display = "block";
                        // scannerImgFrame.style.display = "block";
                    }
                    // window.appOverlay.style.borderColor = "rgb(62, 78, 184)"; // New CSS handles overlay visuals
                    displayScannerUI(!0); // Scan in image mode
                }
            }))
        })() // IIFE for file input setup
    }));

    closeDialogButton.addEventListener("click", resetAndPrepareForScan, !1);

    // REMOVE "About" Dialog close button listener
    // infoDialogCloseButton && infoDialogCloseButton.addEventListener("click", (function() {
    //     infoDialog.classList.add("app__infodialog--hide");
    //     infoDialogOverlay.classList.add("app__infodialog--hide");
    // }), !1);

    openLinkButton.addEventListener("click", (function() {
        var urlToOpen = scannedQrData; // Renamed 'e' to 'urlToOpen'
        // URL prefix check (existing)
        ((e = "") => new RegExp("^(https?:\\/\\/)", "i").test(e))(urlToOpen) || (urlToOpen = `//${urlToOpen}`);

        window.open(urlToOpen, "_blank", "toolbar=0,location=0,menubar=0");
        scannedQrData = null;
        resetAndPrepareForScan();
    }), !1);

    // CHANGE Header Icon listener to RELOAD
    headerIconElement && headerIconElement.addEventListener("click", (function() {
        // OLD "About" dialog logic:
        // infoDialog.classList.remove("app__infodialog--hide");
        // infoDialogOverlay.classList.remove("app__infodialog--hide");
        // NEW Reload logic:
        location.reload();
    }), !1);

}));
