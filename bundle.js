"use strict";
// Snackbar utility (e object)
var e = {},
    t = document.querySelector(".app__snackbar"),
    a = null;
e.show = (e, o = 4e3) => {
    e && (a && a.remove(), (a = document.createElement("div")).className = "app__snackbar-msg", a.textContent = e, t.appendChild(a), setTimeout((() => {
        // Ensure 'a' (the snackbar message element) still exists before trying to remove
        if (a && a.parentNode === t) {
            a.remove();
        }
        // Check if snackbar container is empty, if so, hide it based on new CSS
        if (t.children.length === 0) {
            t.classList.add('app__snackbar--hide');
        } else {
            t.classList.remove('app__snackbar--hide');
        }
    }), o));
    // If adding a new message, ensure container is visible
    if (e && t.children.length > 0) {
        t.classList.remove('app__snackbar--hide');
    }
};

// Camera and scanning logic (o object)
var o = {};

function i(e) { // Renamed original 's' function to 'getCameraElement' for clarity
    !e && window.isMediaStreamAPISupported ? o.webcam = document.querySelector("video") : o.webcam = document.querySelector("img")
}
o.active = !1, o.webcam = null, o.canvas = null, o.ctx = null, o.decoder = null, o.setCanvas = () => {
    o.canvas = document.createElement("canvas"), o.ctx = o.canvas.getContext("2d", { willReadFrequently: true }) // Added willReadFrequently
}, o.init = () => {
    var t = !1; // hasBeenSetup

    function a() { // setupCanvasDimensions
        // Adjust to use video dimensions if available, otherwise window dimensions
        if (o.webcam && o.webcam.videoWidth && o.webcam.videoHeight) {
            o.canvas.width = o.webcam.videoWidth;
            o.canvas.height = o.webcam.videoHeight;
        } else {
            o.canvas.width = window.innerWidth; // Fallback, might not be ideal for image input
            o.canvas.height = window.innerHeight;
        }
    }

    function n(e) { // startStream
        navigator.mediaDevices.getUserMedia(e).then((function(e) {
            o.webcam.srcObject = e, o.webcam.setAttribute("playsinline", !0), o.webcam.setAttribute("controls", !0), setTimeout((() => {
                // Ensure video element exists before trying to remove controls
                const videoEl = document.querySelector("video");
                if (videoEl) videoEl.removeAttribute("controls");
            }))
        })).catch((function(e) {
            console.log("Error occurred while starting stream: ", e), d() // Call handleCameraError
        }))
    }

    function d() { // handleCameraError
        window.noCameraPermission = !0;
        const customScanner = document.querySelector(".custom-scanner");
        if (customScanner) customScanner.style.display = "none";
        // Optionally hide the main scanner frame too if camera fails critically
        // const appOverlayAfter = document.querySelector(".app__overlay::after"); // This won't work for pseudo-elements
        // Consider hiding window.appOverlay itself or a specific visual class
        e.show("Unable to access the camera. Try selecting a photo.", 1e4)
    }
    i(); // Initialize o.webcam
    o.setCanvas();
    try {
        o.decoder = new Worker("decoder.js"); // Ensure decoder.js is in the correct path
    } catch (workerError) {
        console.error("Failed to create Web Worker for decoder.js:", workerError);
        e.show("QR Decoder could not be initialized.", 10000);
        return; // Stop initialization if worker fails
    }


    if (window.isMediaStreamAPISupported) {
        if (o.webcam && o.webcam.tagName === "VIDEO") { // Check if it's actually a video element
             o.webcam.addEventListener("play", (function(e) {
                if (!t) { // if !hasBeenSetup
                    a(); // setupCanvasDimensions
                    t = !0; // hasBeenSetup = true
                }
            }), !1);
        } else { // Fallback if o.webcam is an img or not found
            a(); // setupCanvasDimensions (will use window size)
        }
    } else {
        a(); // setupCanvasDimensions (will use window size if not mediastream)
    }


    window.isMediaStreamAPISupported && navigator.mediaDevices.enumerateDevices().then((function(e) {
        var t, a = e.filter((function(e) {
            // if (e.label.split(",")[1], "videoinput" == e.kind) return e // Original label check might fail if label is empty
            if ("videoinput" === e.kind) return e;
            return false;
        }));
        if (a.length > 1) { // Prefer rear camera
            const rearCamera = a.find(device => device.label && device.label.toLowerCase().includes('back')) || a[a.length - 1];
            t = {
                video: {
                    deviceId: { exact: rearCamera.deviceId }
                },
                audio: !1
            };
            if (window.iOS) t.video.facingMode = "environment";
            n(t); // startStream
        } else if (a.length) {
            t = {
                video: {
                    deviceId: { exact: a[0].deviceId }
                },
                audio: !1
            };
            if (window.iOS) t.video.facingMode = "environment";
            n(t); // startStream
        } else {
            n({ // Fallback to default video input
                video: { facingMode: "environment" } // Try for environment first
            });
        }
    })).catch((function(e) {
        d(), console.error("Error enumerating devices: ", e) // Call handleCameraError
    }))
}, o.scan = function(scanCallback, isFromFile = false) { // Renamed 'e' to 'scanCallback', 't' to 'isFromFile'
    function a() { // tick
        if (o.active) try {
            // Ensure webcam is ready and has dimensions before drawing
            if ((o.webcam.videoWidth && o.webcam.videoHeight && o.webcam.tagName === "VIDEO") || (o.webcam.naturalWidth && o.webcam.naturalHeight && o.webcam.tagName === "IMG")){
                // Adjust canvas size to match current source (video or image)
                if (o.webcam.tagName === "VIDEO") {
                    o.canvas.width = o.webcam.videoWidth;
                    o.canvas.height = o.webcam.videoHeight;
                } else if (o.webcam.tagName === "IMG") {
                    o.canvas.width = o.webcam.naturalWidth;
                    o.canvas.height = o.webcam.naturalHeight;
                }

                o.ctx.drawImage(o.webcam, 0, 0, o.canvas.width, o.canvas.height);
                var e = o.ctx.getImageData(0, 0, o.canvas.width, o.canvas.height);
                e.data && o.decoder && o.decoder.postMessage(e) // Check if o.decoder exists
            } else if (o.webcam.tagName === "VIDEO" && o.webcam.readyState < 2) {
                 // Video not ready, try again shortly
            } else {
                // console.warn("Webcam not ready or dimensions unavailable for drawing.");
            }
            // Continue scanning only if active
            if (o.active) {
                requestAnimationFrame(a); // Use requestAnimationFrame for smoother looping
            }

        } catch (e) {
            console.error("Error in scanning tick:", e);
            if ("NS_ERROR_NOT_AVAILABLE" == e.name && o.active) { // Check o.active before retrying
                // Do not call setTimeout(a,0) here directly, rely on requestAnimationFrame
            } else if (o.active) {
                // For other errors, perhaps stop scanning or log more, but still schedule next frame if active
                // requestAnimationFrame(a);
            }
        }
    }
    o.active = !0;
    // o.setCanvas(); // Canvas is already set in init and potentially resized in tick if needed
    i(isFromFile); // Set o.webcam to video or img based on context

    if (o.decoder) { // Ensure decoder worker exists
        o.decoder.onmessage = function(t) {
            if (t.data && t.data.length > 0) { // Decoder.js seems to send an array of results
                var i = t.data[0][2]; // Assuming data structure [detection, type, content]
                o.active = !1; // Stop scanning loop
                scanCallback(i);
            }
            // No need to call tick (a) here, it's handled by requestAnimationFrame
            // if (o.active) requestAnimationFrame(a); // Only if you want to continue scanning after a non-match
        };
    } else {
        console.error("Decoder worker not initialized.");
        e.show("QR Decoder not available.", 5000);
        return;
    }

    // Start the scanning loop
    requestAnimationFrame(a);
};

// Service Worker
"serviceWorker" in navigator && window.addEventListener("load", (() => {
    navigator.serviceWorker.register("/service-worker.js") // Ensure this path is correct
        .then((t => {
            localStorage.getItem("offline") || (localStorage.setItem("offline", !0), e.show("App is ready for offline usage.", 5e3))
        })).catch((e => {
            console.log("SW registration failed: ", e)
        }))
}));

// DOMContentLoaded - Main App Logic
window.addEventListener("DOMContentLoaded", (() => {
    window.iOS = ["iPad", "iPhone", "iPod"].indexOf(navigator.platform) >= 0;
    window.isMediaStreamAPISupported = navigator && navigator.mediaDevices && "enumerateDevices" in navigator.mediaDevices;
    window.noCameraPermission = !1;

    var qrCodeValue = null; // Renamed 'e'
    var imageElement = null; // Renamed 't' (used for file input image preview)

    const selectPhotosButton = document.querySelector(".app__select-photos");
    const resultDialog = document.querySelector(".app__dialog");
    const resultDialogOverlay = document.querySelector(".app__dialog-overlay");
    const openLinkButton = document.querySelector(".app__dialog-open");
    const closeDialogButton = document.querySelector(".app__dialog-close");
    const customScannerLine = document.querySelector(".custom-scanner");
    // const scannerImgOverlay = document.querySelector(".app__scanner-img"); // This is hidden by new CSS
    const resultInput = document.querySelector("#result");
    const helpText = document.querySelector(".app__help-text"); // Grab help text element
    // const infoIcon = document.querySelector(".app__header-icon svg"); // Not needed for info
    const videoEl = document.querySelector("video"); // Explicitly get video

    const reloadButton = document.getElementById("reloadAppButton"); // Get the reload button

    // REMOVE Info Dialog elements and their listeners
    // const infoDialog = document.querySelector(".app__infodialog");
    // const infoDialogCloseButton = document.querySelector(".app__infodialog-close");
    // const infoDialogOverlay = document.querySelector(".app__infodialog-overlay");

    function updateHelpText(newText) {
        if (helpText) helpText.textContent = newText;
    }

    function showScannerVisuals(show = true) {
        if (customScannerLine) customScannerLine.style.display = show ? "block" : "none";
        // scannerImgOverlay is hidden by CSS, so no need to manage its display here.
        // The main .app__overlay::after provides the border now.
    }


    function startScanProcess(isFromFile = !1) {
        if (window.isMediaStreamAPISupported && !window.noCameraPermission && !isFromFile) {
            updateHelpText("Point camera at a QR Code");
            showScannerVisuals(true);
        } else if (isFromFile) {
            updateHelpText("Processing image...");
            showScannerVisuals(true); // Show scanner line while processing image
        } else {
            updateHelpText("Camera unavailable. Select a photo.");
            showScannerVisuals(false);
        }

        o.scan(((scannedData) => { // Renamed 't' to 'scannedData'
            qrCodeValue = scannedData;
            resultInput.value = scannedData;
            resultInput.select();
            showScannerVisuals(false);

            // URL Check
            if (((url = "") => !(!url || "string" != typeof url) && new RegExp("^(https?:\\/\\/)?((([a-z\\d]([a-z\\d-]*[a-z\\d])*)\\.)+[a-z]{2,}|((\\d{1,3}\\.){3}\\d{1,3}))(\\:\\d+)?(\\/[-a-z\\d%_.~+]*)*(\\?[;&a-z\\d%_.~+=-]*)?(\\#[-a-z\\d_]*)?$", "i").test(url))(scannedData)) {
                openLinkButton.style.display = "inline-flex"; // Or "flex"
            } else {
                openLinkButton.style.display = "none";
            }
            resultDialog.classList.remove("app__dialog--hide");
            resultDialogOverlay.classList.remove("app__dialog--hide");
            updateHelpText("Scan complete!");
        }), isFromFile);
    }

    function resetAppAfterDialog() {
        qrCodeValue = null;
        resultInput.value = "";
        openLinkButton.style.display = "none"; // Hide open button
        if (!window.isMediaStreamAPISupported && imageElement) { // If it was an image scan
            imageElement.src = "";
            imageElement.className = ""; // Reset class if any was added
        }
        resultDialog.classList.add("app__dialog--hide");
        resultDialogOverlay.classList.add("app__dialog--hide");
        startScanProcess(); // Restart scanning with camera if available
    }

    window.appOverlay = document.querySelector(".app__overlay"); // Used for border style in original

    window.addEventListener("load", (e => {
        o.init(); // Initialize camera and decoder worker
        setTimeout((() => {
            if (window.appOverlay) window.appOverlay.style.borderStyle = "solid"; // Original style, CSS now handles visual frame
            if (window.isMediaStreamAPISupported && !window.noCameraPermission) {
                startScanProcess();
            } else if (!window.isMediaStreamAPISupported) {
                updateHelpText("Camera not supported. Select a photo.");
                showScannerVisuals(false);
            } else if (window.noCameraPermission) {
                 updateHelpText("Camera permission denied. Select a photo.");
                 showScannerVisuals(false);
            }
        }), 1e3); // Delay to allow camera init

        // File input setup
        (function() {
            var fileInput = document.createElement("input");
            fileInput.setAttribute("type", "file");
            fileInput.setAttribute("accept", "image/*"); // More specific for images
            // fileInput.setAttribute("capture", "camera"); // 'capture' is more for direct camera launch, not always desired for "select photo"
            fileInput.id = "camera"; // Retain original ID if other parts rely on it, though it's a bit misleading now
            // window.appOverlay.style.borderStyle = ""; // Original, not strictly needed with new CSS

            if (selectPhotosButton) selectPhotosButton.style.display = "flex"; // Ensure FAB is visible

            imageElement = document.createElement("img"); // This is 't' from original
            imageElement.src = "";
            imageElement.id = "frame"; // Retain original ID
            imageElement.style.display = "none"; // Hide the preview img element itself, we use canvas

            var layoutContent = document.querySelector(".app__layout-content");
            if (layoutContent) {
                layoutContent.appendChild(fileInput);
                layoutContent.appendChild(imageElement);
            }


            if (selectPhotosButton) {
                selectPhotosButton.addEventListener("click", (() => {
                    o.active = false; // Stop camera scanning if active
                    // showScannerVisuals(false); // Hide camera scanner line
                    fileInput.click(); // Open file dialog
                }));
            }

            fileInput.addEventListener("change", (e => {
                if (e.target && e.target.files && e.target.files.length > 0) {
                    // o.webcam is now the img element for file scanning
                    // The 'i(isFromFile)' call inside o.scan will handle setting o.webcam = imageElement
                    imageElement.src = URL.createObjectURL(e.target.files[0]);
                    imageElement.onload = () => { // Wait for image to load before scanning
                        // window.appOverlay.style.borderColor = "rgb(62, 78, 184)"; // Original style
                        startScanProcess(true); // True for isFromFile
                        URL.revokeObjectURL(imageElement.src); // Clean up object URL after load
                    };
                    imageElement.onerror = () => {
                        e.show("Failed to load image.", 4000);
                        updateHelpText("Error loading image. Try again.");
                    };
                }
                fileInput.value = ""; // Reset file input
            }));
        })() // IIFE for file input setup
    }));

    if (closeDialogButton) closeDialogButton.addEventListener("click", resetAppAfterDialog, !1);

    // REMOVE Info Dialog Listeners
    // if (infoDialogCloseButton) infoDialogCloseButton.addEventListener("click", (function() {
    //     if(infoDialog) infoDialog.classList.add("app__infodialog--hide");
    //     if(infoDialogOverlay) infoDialogOverlay.classList.add("app__infodialog--hide");
    // }), !1);

    if (openLinkButton) openLinkButton.addEventListener("click", (function() {
        if (!qrCodeValue) return;
        let urlToOpen = qrCodeValue;
        if (!((u = "") => new RegExp("^(https?:\\/\\/)", "i").test(u))(urlToOpen)) {
            urlToOpen = `//${urlToOpen}`; // Prepend // if no http/https
        }
        window.open(urlToOpen, "_blank", "noopener,noreferrer,toolbar=0,location=0,menubar=0");
        qrCodeValue = null;
        resetAppAfterDialog();
    }), !1);

    // Modify header icon to be RELOAD button
    if (reloadButton) { // Changed from 'u' (original info icon container)
        reloadButton.addEventListener("click", (function() {
            // REMOVE Info Dialog show logic
            // if(infoDialog) infoDialog.classList.remove("app__infodialog--hide");
            // if(infoDialogOverlay) infoDialogOverlay.classList.remove("app__infodialog--hide");

            // ADD Reload logic
            location.reload();
        }), !1);
    }
}));
