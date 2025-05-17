"use strict";

// Snackbar utility (original)
var e = {}, // Renamed from 'e' to 'Snackbar' for clarity
    snackbarElement = document.querySelector(".app__snackbar"), // Renamed from 't'
    currentSnackbarMessage = null; // Renamed from 'a'
e.show = (messageText, duration = 4e3) => { // Renamed params
    if (!messageText) return;
    currentSnackbarMessage && currentSnackbarMessage.remove();
    currentSnackbarMessage = document.createElement("div");
    currentSnackbarMessage.className = "app__snackbar-msg";
    currentSnackbarMessage.textContent = messageText;
    snackbarElement.appendChild(currentSnackbarMessage);

    // --- AI Style Snackbar Animation ---
    // Force reflow for animation
    void currentSnackbarMessage.offsetWidth;
    // Assuming CSS handles entry: .app__snackbar:not(.app__snackbar--hide) .app__snackbar-msg { opacity: 1; transform: ...; }
    // And snackbarContainer has .app__snackbar--hide removed before this.
    snackbarElement.classList.remove('app__snackbar--hide');


    setTimeout(() => {
        // --- AI Style Snackbar Exit ---
        if (currentSnackbarMessage) {
            currentSnackbarMessage.style.opacity = '0';
            currentSnackbarMessage.style.transform = 'translateY(25px) scale(0.85)'; // Match CSS exit
            setTimeout(() => {
                if (currentSnackbarMessage) currentSnackbarMessage.remove();
                currentSnackbarMessage = null;
                if (snackbarElement.children.length === 0) {
                    snackbarElement.classList.add('app__snackbar--hide');
                }
            }, 300); // Match CSS transition duration for exit
        }
    }, duration);
};


var o = {}; // Camera and Scan logic (original)
function i(e) {
    !e && window.isMediaStreamAPISupported ? o.webcam = document.querySelector("video") : o.webcam = document.querySelector("img")
}
o.active = !1, o.webcam = null, o.canvas = null, o.ctx = null, o.decoder = null, o.setCanvas = () => {
    o.canvas = document.createElement("canvas"), o.ctx = o.canvas.getContext("2d", { willReadFrequently: true }) // Added willReadFrequently
}, o.init = () => {
    var t = !1;

    function a() { // Renamed from 'a' to 'setCanvasDimensions' for clarity
        // Use video dimensions if available and playing, otherwise window dimensions
        if (o.webcam && o.webcam.videoWidth && o.webcam.videoHeight) {
            o.canvas.width = o.webcam.videoWidth;
            o.canvas.height = o.webcam.videoHeight;
        } else {
            o.canvas.width = window.innerWidth; // Fallback, less ideal for QR
            o.canvas.height = window.innerHeight;
        }
    }

    function n(e) { // Renamed from 'n' to 'getUserMediaSuccess'
        navigator.mediaDevices.getUserMedia(e).then((function(stream) { // Renamed 'e' to 'stream'
            o.webcam.srcObject = stream;
            o.webcam.setAttribute("playsinline", !0);
            o.webcam.setAttribute("controls", !0); // Temporarily add controls for browsers that need it
            setTimeout((() => {
                // Remove controls once playback starts or if not needed
                if (document.querySelector("video")) {
                     document.querySelector("video").removeAttribute("controls");
                }
            }))
            o.webcam.play().catch(e => { // Add play() and catch potential errors
                console.error("Video play error:", e);
                d("Error playing video stream.");
            });
        })).catch((function(e) {
            console.log("Error occurred getting user media ", e);
            d() // Call general camera error
        }))
    }

    function d(customMessage) { // Renamed from 'd' to 'handleCameraError'
        window.noCameraPermission = !0;
        const scannerLine = document.querySelector(".custom-scanner");
        if (scannerLine) scannerLine.style.display = "none";
        e.show(customMessage || "Unable to access the camera. Please check permissions.", 1e4);
        // --- NEW: Update help text on camera error ---
        const helpText = document.querySelector(".app__help-text");
        if (helpText) helpText.textContent = "Camera error. Try selecting a photo.";
    }
    i(), o.setCanvas(), o.decoder = new Worker("decoder.js"); // Assuming decoder.js is correctly set up as a Worker
    if (window.isMediaStreamAPISupported) {
        o.webcam.addEventListener("loadedmetadata", (function(e) { // Use loadedmetadata instead of play for dimensions
            if (!t) {
                a(); // Set canvas dimensions based on video
                t = !0
            }
        }), !1);
        o.webcam.addEventListener("play", (function(e) {
            // Optional: Can also call a() here if dimensions might change after play starts
            // if (!t) { a(); t = !0; } // Original logic
        }), !1);
    } else {
       a(); // For image fallback
    }

    if (window.isMediaStreamAPISupported && navigator.mediaDevices.enumerateDevices) {
        navigator.mediaDevices.enumerateDevices().then((function(e) { // 'e' is devices list
            var t, a = e.filter((function(device) { // Renamed 'e' to 'device'
                if (device.label.split(",")[1], "videoinput" == device.kind) return device
            }));
            if (a.length > 1) {
                // Prefer rear camera (often last in the list or contains 'back' or 'environment')
                let rearCamera = a.find(device => device.label.toLowerCase().includes('back') || device.label.toLowerCase().includes('environment'));
                if (!rearCamera) rearCamera = a[a.length - 1]; // Fallback to last one

                t = {
                    video: {
                        deviceId: { exact: rearCamera.deviceId }
                    },
                    audio: !1
                };
                 // iOS specific facingMode is good, but deviceId is more specific if available
                if (window.iOS && !rearCamera.deviceId) t.video.facingMode = "environment";
                n(t);

            } else if (a.length) {
                t = {
                    video: {
                        deviceId: { exact: a[0].deviceId }
                    },
                    audio: !1
                };
                if (window.iOS && !a[0].deviceId) t.video.facingMode = "environment";
                n(t);
            } else {
                n({ video: { facingMode: "environment" } }) // Fallback if no specific device found
            }
        })).catch((function(e) {
            d(); // General camera error
            console.error("Error enumerating devices : ", e)
        }))
    } else if (window.isMediaStreamAPISupported) {
        // Fallback for browsers that support gUM but not enumerateDevices (rare)
        n({ video: { facingMode: "environment" } });
    } else {
        // No MediaStream API support (handled by image upload)
        document.querySelector(".custom-scanner").style.display = "none";
        const helpText = document.querySelector(".app__help-text");
        if (helpText) helpText.textContent = "Camera not supported. Please select an image.";
    }
}, o.scan = function(scanSuccessCallback, isImageScan) { // Renamed 'e' to 'scanSuccessCallback', 't' to 'isImageScan'
    function attemptScan() { // Renamed 'a' to 'attemptScan'
        if (o.active) try {
            // Ensure canvas dimensions are set correctly before drawing
            if (o.webcam && (o.webcam.videoWidth || o.webcam.naturalWidth)) { // Check for video or img dimensions
                 o.canvas.width = isImageScan ? o.webcam.naturalWidth : o.webcam.videoWidth;
                 o.canvas.height = isImageScan ? o.webcam.naturalHeight : o.webcam.videoHeight;
            } else if (!isImageScan) { // Fallback for video if dimensions not ready (should not happen often with loadedmetadata)
                o.canvas.width = window.innerWidth;
                o.canvas.height = window.innerHeight;
            }


            o.ctx.drawImage(o.webcam, 0, 0, o.canvas.width, o.canvas.height);
            var imageData = o.ctx.getImageData(0, 0, o.canvas.width, o.canvas.height); // Renamed 'e' to 'imageData'
            if (imageData.data && imageData.data.length > 0) { // Check if data is not empty
                o.decoder.postMessage(imageData) // Send the whole imageData object
            } else if (!isImageScan) { // If it's video and data is empty, try again
                 setTimeout(attemptScan, 100); // Retry with a small delay
            }
        } catch (e) {
            // "NS_ERROR_NOT_AVAILABLE" may occur if the video is not yet ready or dimensions are 0.
            if ("NS_ERROR_NOT_AVAILABLE" == e.name && !isImageScan) {
                console.warn("NS_ERROR_NOT_AVAILABLE, retrying scan for video.");
                setTimeout(attemptScan, 100); // Retry with a small delay
            } else {
                console.error("Error in scan loop:", e);
                // Optionally, stop scanning on other errors for image scans
                // if (isImageScan) o.active = false;
            }
        }
    }
    o.active = !0;
    // o.setCanvas(); // Canvas is already set in o.init and dimensions updated in attemptScan

    o.decoder.onmessage = function(t) { // 't' is the message from worker
        if (t.data && t.data.length > 0 && t.data[0] && t.data[0][2]) { // Check structure from original code
            var i = t.data[0][2]; // The QR data string
            o.active = !1; // Stop scanning loop
            scanSuccessCallback(i);
        } else if (o.active && !isImageScan) { // If no data and it's video scan, continue
            setTimeout(attemptScan, 0); // Continue scanning video
        } else if (o.active && isImageScan) { // If no data and it's image scan, it means no QR found
            o.active = !1;
            scanSuccessCallback(null); // Indicate no QR found for image
        }
    };

    setTimeout((() => {
        i(isImageScan) // Update o.webcam reference if needed (img vs video)
    }));
    attemptScan(); // Start the scan loop
};

// Service Worker (original)
"serviceWorker" in navigator && window.addEventListener("load", (() => {
    navigator.serviceWorker.register("/service-worker.js").then((t => {
        localStorage.getItem("offline") || (localStorage.setItem("offline", !0), e.show("App is ready for offline usage.", 5e3))
    })).catch((e => {
        console.log("SW registration failed: ", e)
    }))
}));


window.addEventListener("DOMContentLoaded", (() => {
    window.iOS = ["iPad", "iPhone", "iPod"].indexOf(navigator.platform) >= 0;
    window.isMediaStreamAPISupported = navigator && navigator.mediaDevices && "enumerateDevices" in navigator.mediaDevices;
    window.noCameraPermission = !1;

    var qrCodeData = null; // Renamed 'e' to 'qrCodeData'
    var imageElementForScan = null; // Renamed 't' to 'imageElementForScan' (this is the <img> tag)

    const selectPhotosButton = document.querySelector(".app__select-photos"); // Renamed 'a'
    const resultDialog = document.querySelector(".app__dialog"); // Renamed 'i'
    const resultDialogOverlay = document.querySelector(".app__dialog-overlay"); // Renamed 'n'
    const openLinkButton = document.querySelector(".app__dialog-open"); // Renamed 'd'
    const closeDialogButton = document.querySelector(".app__dialog-close"); // Renamed 'r'
    const scannerLine = document.querySelector(".custom-scanner"); // Renamed 'c'
    const scannerImgFrame = document.querySelector(".app__scanner-img"); // Renamed 'l' (though it's hidden by new CSS)
    const resultInputElement = document.querySelector("#result"); // Renamed 's'
    const helpTextElement = document.querySelector(".app__help-text");
    // const infoIcon = document.querySelector(".app__header-icon svg"); // No longer info icon
    const videoElement = document.querySelector("video"); // Already referenced by o.webcam for video mode

    // --- MODIFICATION: Get Reload button instead of Info Dialog elements ---
    const reloadAppButton = document.getElementById("reloadAppButton"); // Was 'u' (header icon wrapper)
    // const infoDialog = document.querySelector(".app__infodialog"); // REMOVED (was 'p')
    // const infoDialogCloseButton = document.querySelector(".app__infodialog-close"); // REMOVED (was 'm')
    // const infoDialogOverlay = document.querySelector(".app__infodialog-overlay"); // REMOVED (was 'v')

    // --- NEW: Reload button event listener ---
    if (reloadAppButton) {
        reloadAppButton.addEventListener('click', function() {
            location.reload();
        });
    }


    function startScanProcess(isImageScan = !1) { // Renamed 'y' to 'startScanProcess', 't' to 'isImageScan'
        if (helpTextElement) helpTextElement.textContent = isImageScan ? "Processing image..." : "Point camera at a QR Code";

        if (window.isMediaStreamAPISupported && !window.noCameraPermission && !isImageScan) {
            if (scannerLine) scannerLine.style.display = "block";
            if (scannerImgFrame) scannerImgFrame.style.display = "block"; // Though hidden by AI CSS
        } else if (isImageScan) {
            // For image scan, scanner line visibility might depend on if visual feedback is desired
            if (scannerLine) scannerLine.style.display = "none"; // Typically hidden for static image
            if (scannerImgFrame) scannerImgFrame.style.display = "none";
        }


        o.scan(((qrData) => { // Renamed 't' to 'qrData'
            qrCodeData = qrData; // Store scanned data

            if (scannerLine) scannerLine.style.display = "none";
            if (scannerImgFrame) scannerImgFrame.style.display = "none";

            if (qrData) { // If QR data is found
                resultInputElement.value = qrData;
                resultInputElement.select(); // For easy copying

                // URL Check (original, slightly improved)
                const isURL = ((urlStr = "") => {
                    if (!urlStr || typeof urlStr !== "string") return false;
                    // Simplified regex, but good enough for many cases. For robust, use URL API.
                    try {
                        new URL(urlStr); // Check if it can be parsed as a URL
                        // Further check common protocols if needed, but URL API handles a lot
                        return /^(https?:|ftp:|file:)/i.test(urlStr) || (!urlStr.includes(':') && urlStr.includes('.'));
                    } catch (_) {
                         // Check for common patterns if URL parsing fails (e.g. domain.com without protocol)
                        return !urlStr.includes(':') && urlStr.includes('.') && urlStr.length > 3;
                    }
                })(qrData);

                if (isURL) {
                    openLinkButton.style.display = "inline-flex"; // Match new CSS
                } else {
                    openLinkButton.style.display = "none";
                }
                resultDialog.classList.remove("app__dialog--hide");
                resultDialogOverlay.classList.remove("app__dialog--hide");
                if (helpTextElement) helpTextElement.textContent = "Scan complete!";
                 if (navigator.vibrate) navigator.vibrate(100); // Vibrate on success
            } else if (isImageScan) { // No QR found in image
                e.show("No QR code found in the selected image.", 3000);
                if (helpTextElement) helpTextElement.textContent = "No QR code found. Try another or use camera.";
                 // Restart camera if it was active and stopped for image scan
                if (!o.active && window.isMediaStreamAPISupported && !window.noCameraPermission) {
                    setTimeout(() => o.init(), 100); // Re-init camera
                    setTimeout(() => startScanProcess(false), 1100); // Start scanning video
                }
            }
        }), isImageScan)
    }

    function resetAppUI() { // Renamed 'w' to 'resetAppUI'
        qrCodeData = null;
        resultInputElement.value = "";
        openLinkButton.style.display = "none"; // Hide open button

        if (!window.isMediaStreamAPISupported && imageElementForScan) { // If it was an image scan
            imageElementForScan.src = "";
            imageElementForScan.className = "";
        }
        resultDialog.classList.add("app__dialog--hide");
        resultDialogOverlay.classList.add("app__dialog--hide");

        // Restart video scan if camera is supported and permission granted
        if (window.isMediaStreamAPISupported && !window.noCameraPermission) {
            // Ensure camera is initialized if it was stopped
            if (!o.webcam || !o.webcam.srcObject || o.webcam.paused) {
                setTimeout(() => o.init(), 100); // Re-init camera
                setTimeout(() => startScanProcess(false), 1100); // Start scanning video
            } else {
                 startScanProcess(false); // Directly start if camera seems ready
            }
        } else if (helpTextElement) {
            helpTextElement.textContent = window.noCameraPermission ? "Camera permission denied. Select photo." : "Camera not supported. Select photo.";
        }
    }

    window.appOverlay = document.querySelector(".app__overlay"); // Used by original CSS

    window.addEventListener("load", (event => { // Renamed 'e' to 'event'
        o.init(); // Initialize camera and scanner logic
        setTimeout((() => {
            // window.appOverlay.style.borderStyle = "solid"; // Original, not needed with new CSS
            if (window.isMediaStreamAPISupported && !window.noCameraPermission) {
                startScanProcess(false); // Start video scan
            } else if (helpTextElement) {
                helpTextElement.textContent = window.noCameraPermission ? "Camera permission denied. Select photo." : "Camera not supported. Select photo.";
            }
        }), 1000); // Delay to allow camera init


        // File input setup (original)
        (function() {
            var fileInput = document.createElement("input"); // Renamed 'e' to 'fileInput'
            fileInput.setAttribute("type", "file");
            fileInput.setAttribute("accept", "image/*"); // Added accept attribute
            // fileInput.setAttribute("capture", "camera"); // 'capture' is more for direct camera input
            fileInput.id = "camera"; // Original ID, though it's a file picker now
            // window.appOverlay.style.borderStyle = ""; // Original, not needed

            selectPhotosButton.style.display = "flex"; // Ensure FAB is visible

            imageElementForScan = document.createElement("img"); // This is 't' from original
            imageElementForScan.src = "";
            imageElementForScan.id = "frame"; // This is the <img> that will hold the selected image

            var layoutContent = document.querySelector(".app__layout-content");
            layoutContent.appendChild(fileInput); // Hidden by CSS
            layoutContent.appendChild(imageElementForScan); // Hidden by CSS unless class is added

            selectPhotosButton.addEventListener("click", (() => {
                if (scannerLine) scannerLine.style.display = "none"; // Hide scanner line for file picking
                if (scannerImgFrame) scannerImgFrame.style.display = "none";
                o.active = false; // Stop any active video scanning
                if (o.webcam && o.webcam.srcObject && typeof o.webcam.srcObject.getTracks === 'function') { // Stop video stream
                    o.webcam.srcObject.getTracks().forEach(track => track.stop());
                    o.webcam.srcObject = null;
                }
                fileInput.click()
            }));

            fileInput.addEventListener("change", (event => { // Renamed 'e' to 'event'
                if (event.target && event.target.files.length > 0) {
                    const selectedFile = event.target.files[0];
                    imageElementForScan.className = "app__overlay"; // To give it dimensions, though it's hidden
                    imageElementForScan.src = URL.createObjectURL(selectedFile);

                    imageElementForScan.onload = () => { // Wait for image to load to get dimensions
                        // window.noCameraPermission || (scannerLine.style.display = "block", scannerImgFrame.style.display = "block"); // Original logic
                        // window.appOverlay.style.borderColor = "rgb(62, 78, 184)"; // Original
                        i(true); // Set o.webcam to the img element for scanning
                        startScanProcess(!0); // True for isImageScan
                        URL.revokeObjectURL(imageElementForScan.src); // Clean up object URL after load
                    };
                    imageElementForScan.onerror = () => {
                        e.show("Failed to load selected image.", 3000);
                         if (helpTextElement) helpTextElement.textContent = "Error loading image. Try again.";
                    };
                    fileInput.value = ''; // Reset file input
                }
            }))
        })() // IIFE for file input setup
    }));

    closeDialogButton.addEventListener("click", resetAppUI, !1);

    // --- REMOVED Info Dialog Listeners ---
    // infoDialogCloseButton.addEventListener("click", (function() {
    //     infoDialog.classList.add("app__infodialog--hide");
    //     infoDialogOverlay.classList.add("app__infodialog--hide");
    // }), !1);
    // reloadAppButton.addEventListener("click", (function() { // Was 'u' which was the info icon wrapper
    //     infoDialog.classList.remove("app__infodialog--hide");
    //     infoDialogOverlay.classList.remove("app__infodialog--hide");
    // }), !1);


    openLinkButton.addEventListener("click", (function() {
        let urlToOpen = qrCodeData; // Renamed 'e' to 'urlToOpen'
        // URL Prepending logic (original, slightly safer)
        if (urlToOpen && !new RegExp("^(?:f|ht)tps?:\\/\\/", "i").test(urlToOpen)) {
            urlToOpen = `//${urlToOpen}`;
        }
        if (urlToOpen) {
             window.open(urlToOpen, "_blank", "toolbar=0,location=0,menubar=0,noopener,noreferrer");
        }
        qrCodeData = null; // Clear after opening
        resetAppUI(); // Close dialog and reset
    }), !1);

}));
