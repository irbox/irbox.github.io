// bundle.js
document.addEventListener('DOMContentLoaded', function() {

    // --- DOM Elements ---
    const videoElement = document.querySelector('video');
    const selectPhotosButton = document.querySelector('.app__select-photos');
    const resultDialog = document.querySelector('.app__dialog');
    const resultDialogOverlay = document.querySelector('.app__dialog-overlay');
    const resultInputElement = document.getElementById('result');
    const openLinkButton = document.querySelector('.app__dialog-open');
    const closeDialogButton = document.querySelector('.app__dialog-close');
    const snackbarContainer = document.querySelector('.app__snackbar');
    const helpTextElement = document.querySelector('.app__help-text');
    const reloadAppButton = document.getElementById('reloadAppButton');

    // Dynamically create file input
    const fileInputElement = document.createElement('input');
    fileInputElement.type = 'file';
    fileInputElement.accept = 'image/*';
    fileInputElement.style.display = 'none'; // Hidden by CSS, but good practice
    document.body.appendChild(fileInputElement);

    // --- State Variables ---
    let currentStream = null;
    let animationFrameId = null;
    const canvasElement = document.createElement('canvas'); // For processing video frames/images
    const canvasContext = canvasElement.getContext('2d', { willReadFrequently: true });


    // --- Reload Functionality ---
    if (reloadAppButton) {
        reloadAppButton.addEventListener('click', function() {
            location.reload();
        });
    }

    // --- Utility Functions ---
    function setHelpText(text) {
        if (helpTextElement) {
            helpTextElement.textContent = text;
        }
    }

    function showSnackbar(message, duration = 3500) {
        if (!snackbarContainer) return;

        const msgElement = document.createElement('div');
        msgElement.className = 'app__snackbar-msg';
        msgElement.textContent = message;

        while (snackbarContainer.firstChild) {
            snackbarContainer.removeChild(snackbarContainer.firstChild);
        }

        snackbarContainer.appendChild(msgElement);
        snackbarContainer.classList.remove('app__snackbar--hide');

        void msgElement.offsetWidth;

        setTimeout(() => {
            msgElement.style.opacity = '0';
            msgElement.style.transform = 'translateY(25px) scale(0.85)';

            setTimeout(() => {
                if (msgElement.parentNode === snackbarContainer) {
                    snackbarContainer.removeChild(msgElement);
                }
                if (snackbarContainer.children.length === 0) {
                    snackbarContainer.classList.add('app__snackbar--hide');
                }
            }, 300);
        }, duration);
    }

    // --- Dialog Management ---
    function showResultDialog(qrData) {
        if (resultDialog && resultDialogOverlay && resultInputElement) {
            resultInputElement.value = qrData;
            resultDialog.classList.remove('app__dialog--hide');
            resultDialogOverlay.classList.remove('app__dialog--hide');

            if (openLinkButton) {
                try {
                    new URL(qrData);
                    openLinkButton.style.display = 'inline-flex';
                    openLinkButton.onclick = () => {
                        window.open(qrData, '_blank', 'noopener,noreferrer');
                        hideResultDialog();
                    };
                } catch (_) {
                    openLinkButton.style.display = 'none';
                    openLinkButton.onclick = null;
                }
            }
        }
    }

    function hideResultDialog() {
        if (resultDialog && resultDialogOverlay) {
            resultDialog.classList.add('app__dialog--hide');
            resultDialogOverlay.classList.add('app__dialog--hide');
            if (resultInputElement) resultInputElement.value = '';
            if (openLinkButton) openLinkButton.onclick = null;
            if (!currentStream) {
                startCamera();
            }
        }
    }

    if (closeDialogButton) {
        closeDialogButton.addEventListener('click', hideResultDialog);
    }
    if (resultDialogOverlay) {
        resultDialogOverlay.addEventListener('click', (event) => {
            if (event.target === resultDialogOverlay) {
                 hideResultDialog();
            }
        });
    }


    // --- QR Code Decoding ---
    // MODIFIED: This now directly checks for the function from your decoder.js
    // Replace 'window.DecoderJS.decode' with the ACTUAL function name/path provided by decoder.js
    function decodeQrFromImageData(imageDataObject) { // imageDataObject now expected to be { data: Uint8ClampedArray, width: number, height: number }
        // *** IMPORTANT: IDENTIFY THE CORRECT FUNCTION FROM decoder.js ***
        // Example 1: If decoder.js makes 'decode' available on a 'DecoderJS' global object
        if (typeof window.DecoderJS === 'object' && typeof window.DecoderJS.decode === 'function') {
            // Assuming DecoderJS.decode takes (dataArray, width, height)
            const code = window.DecoderJS.decode(imageDataObject.data, imageDataObject.width, imageDataObject.height);
            if (code && code.data) { // Assuming the library returns an object with a 'data' property
                return code.data;
            }
        // Example 2: If decoder.js makes a global function like 'decodeQRCodeFromBuffer'
        } else if (typeof window.decodeQRCodeFromBuffer === 'function') {
             const code = window.decodeQRCodeFromBuffer(imageDataObject.data, imageDataObject.width, imageDataObject.height);
             if (code && code.data) {
                return code.data;
            }
        // Example 3: If decoder.js is jsQR library itself (often named jsQR.js or decoder.js in simple projects)
        } else if (typeof window.jsQR === 'function') {
            const code = window.jsQR(imageDataObject.data, imageDataObject.width, imageDataObject.height, {
                inversionAttempts: "dontInvert",
            });
            if (code && code.data) {
                return code.data;
            }
        } else {
            console.warn("QR decoding function from 'decoder.js' not found or not recognized.");
            console.log("Available on window:", Object.keys(window).filter(key => key.toLowerCase().includes('decode') || key.toLowerCase().includes('qr')));
            showSnackbar("QR Decoder library (decoder.js) not working as expected.", 4000);
        }
        return null;
    }

    function processQrDetection(qrData) {
        if (qrData) {
            console.log("QR Code detected:", qrData);
            showResultDialog(qrData);
            stopCamera();
            setHelpText("Scan complete! Reload or select photo.");
            if (navigator.vibrate) {
                navigator.vibrate(100);
            }
        }
    }

    // --- Camera Handling ---
    function stopCamera() {
        if (currentStream) {
            currentStream.getTracks().forEach(track => track.stop());
            currentStream = null;
        }
        if (animationFrameId) {
            cancelAnimationFrame(animationFrameId);
            animationFrameId = null;
        }
        if (videoElement) {
            videoElement.srcObject = null;
            videoElement.style.opacity = '0';
        }
    }

    async function startCamera() {
        stopCamera();

        setHelpText("Requesting camera access...");
        try {
            currentStream = await navigator.mediaDevices.getUserMedia({
                video: {
                    facingMode: 'environment',
                    width: { ideal: 1280 },
                    height: { ideal: 720 }
                }
            });
            if (videoElement) {
                videoElement.srcObject = currentStream;
                videoElement.onloadedmetadata = () => {
                    videoElement.play();
                    videoElement.style.opacity = '1';
                    setHelpText("Point camera at a QR Code");
                    scanVideoFrame();
                };
                videoElement.onerror = (e) => {
                    console.error("Video element error:", e);
                    showSnackbar("Video playback error.", 4000);
                    stopCamera();
                    setHelpText("Camera error. Try selecting a photo.");
                };
            }
        } catch (err) {
            console.error("Error accessing camera:", err);
            let message = "Could not access camera.";
            if (err.name === "NotAllowedError") {
                message = "Camera permission denied. Please enable it in browser settings.";
            } else if (err.name === "NotFoundError") {
                message = "No camera found. Ensure a camera is connected.";
            } else if (err.name === "NotReadableError") {
                message = "Camera is already in use or unreadable.";
            }
            showSnackbar(message, 5000);
            setHelpText("Camera unavailable. Try selecting a photo.");
            if (videoElement) videoElement.style.opacity = '0';
        }
    }

    function scanVideoFrame() {
        if (!currentStream || !videoElement || videoElement.paused || videoElement.ended) {
            return;
        }

        if (videoElement.readyState === videoElement.HAVE_ENOUGH_DATA) {
            canvasElement.height = videoElement.videoHeight;
            canvasElement.width = videoElement.videoWidth;
            canvasContext.drawImage(videoElement, 0, 0, canvasElement.width, canvasElement.height);
            const imageData = canvasContext.getImageData(0, 0, canvasElement.width, canvasElement.height);
            // Pass the imageData object directly as required by jsQR and similar libraries
            const qrData = decodeQrFromImageData({ data: imageData.data, width: imageData.width, height: imageData.height });
            processQrDetection(qrData);
        }

        if (currentStream) {
            animationFrameId = requestAnimationFrame(scanVideoFrame);
        }
    }

    // --- File Input Handling ---
    if (selectPhotosButton) {
        selectPhotosButton.addEventListener('click', () => {
            if (currentStream) {
                stopCamera();
                setHelpText("Select an image with a QR Code");
            }
            fileInputElement.click();
        });

        fileInputElement.addEventListener('change', (event) => {
            const file = event.target.files[0];
            if (file) {
                setHelpText("Processing image...");
                const reader = new FileReader();
                reader.onload = function(e) {
                    const img = new Image();
                    img.onload = function() {
                        canvasElement.width = img.naturalWidth; // Use naturalWidth for correct dimensions
                        canvasElement.height = img.naturalHeight;
                        canvasContext.drawImage(img, 0, 0, canvasElement.width, canvasElement.height);
                        const imageData = canvasContext.getImageData(0, 0, canvasElement.width, canvasElement.height);
                        const qrData = decodeQrFromImageData({ data: imageData.data, width: imageData.width, height: imageData.height });

                        if (qrData) {
                            processQrDetection(qrData);
                        } else {
                            showSnackbar("No QR code found in the selected image.", 3000);
                            setHelpText("No QR code found. Try another image or use camera.");
                            if (!currentStream) {
                                startCamera();
                            }
                        }
                    };
                    img.onerror = function() {
                        showSnackbar("Could not load the selected image.", 3000);
                        setHelpText("Error loading image. Try again or use camera.");
                        if (!currentStream) startCamera();
                    };
                    img.src = e.target.result;
                };
                reader.onerror = function() {
                    showSnackbar("Could not read the selected file.", 3000);
                    setHelpText("Error reading file. Try again or use camera.");
                     if (!currentStream) startCamera();
                };
                reader.readAsDataURL(file);
                fileInputElement.value = '';
            } else {
                if (!currentStream) startCamera();
            }
        });
    }

    // --- Initial Application Setup ---
    function initializeApp() {
        setHelpText("Initializing scanner...");
        if (navigator.mediaDevices && typeof navigator.mediaDevices.getUserMedia === 'function') {
            startCamera();
        } else {
            showSnackbar("Camera API not supported by this browser.", 5000);
            setHelpText("Camera not supported. Please select an image file.");
        }
    }

    // Start the application
    initializeApp();

}); // End DOMContentLoaded
