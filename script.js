// FFmpeg is loaded globally from the script tag in index.html
// const { createFFmpeg, fetchFile } = FFmpeg; // This line is not strictly needed if FFmpeg is global

document.addEventListener('DOMContentLoaded', async () => {
    // Destructure from global FFmpeg object for convenience
    const { createFFmpeg, fetchFile } = FFmpeg;

    let ffmpeg; // Declare ffmpeg instance variable

    const videoFilesInput = document.getElementById('videoFiles');
    const selectedFileList = document.getElementById('selectedFileList');
    const mergeButton = document.getElementById('mergeButton');
    const clearFilesButton = document.getElementById('clearFiles');
    const progressBar = document.getElementById('progressBar');
    const logOutput = document.getElementById('logOutput');
    const downloadLink = document.getElementById('downloadLink');
    const downloadSection = document.querySelector('.download-section');
    const mergedVideoPreview = document.getElementById('mergedVideoPreview');

    let selectedFilesData = []; // To store File objects in order

    // --- Initialize FFmpeg ---
    log('Loading FFmpeg...');
    mergeButton.disabled = true;
    clearFilesButton.disabled = true;
    try {
        ffmpeg = createFFmpeg({
            // Use the single-threaded core for compatibility (e.g., on GitHub Pages)
            corePath: 'https://unpkg.com/@ffmpeg/core-st@0.11.0/dist/ffmpeg-core.js',
            log: true, // Enable FFmpeg's own logging
        });
        await ffmpeg.load();
        log('FFmpeg loaded successfully.');
        updateButtonStates();
    } catch (error) {
        log(`Error loading FFmpeg: ${error.message}`);
        console.error("FFmpeg loading error:", error);
        alert("Failed to load FFmpeg. Please check your internet connection or browser console for errors. This app might not work on your browser/device.");
        return;
    }

    ffmpeg.setLogger(({ type, message }) => {
        if (type !== 'info' && type !== 'fferr') {
             log(`FFmpeg: ${message}`);
        }
    });

    ffmpeg.setProgress(({ ratio }) => {
        progressBar.style.display = 'block';
        const progressPercent = Math.max(0, Math.min(100, ratio * 100));
        progressBar.value = progressPercent;
        // Avoid flooding logs with progress updates for better readability
        if (progressPercent === 0 || progressPercent === 100 || progressBar.value % 5 === 0) { // Log every 5%
            log(`Progress: ${progressPercent.toFixed(2)}%`);
        }
    });

    // --- File Input Handling ---
    videoFilesInput.addEventListener('change', (event) => {
        const newFiles = Array.from(event.target.files);
        newFiles.forEach(file => {
            if (file.type.startsWith('video/')) {
                if (!selectedFilesData.find(f => f.name === file.name && f.size === file.size)) {
                    selectedFilesData.push(file);
                }
            } else {
                log(`Skipping non-video file: ${file.name}`);
            }
        });
        renderFileList();
        updateButtonStates();
        videoFilesInput.value = '';
    });

    // --- Render File List with Drag-and-Drop Reordering ---
    function renderFileList() {
        selectedFileList.innerHTML = '';
        selectedFilesData.forEach((file, index) => {
            const li = document.createElement('li');
            li.textContent = `${index + 1}. ${file.name} (${(file.size / 1024 / 1024).toFixed(2)} MB)`;
            li.draggable = true;
            li.dataset.index = index;

            const removeBtn = document.createElement('button');
            removeBtn.textContent = 'Remove';
            removeBtn.onclick = (e) => {
                e.stopPropagation();
                selectedFilesData.splice(index, 1);
                renderFileList();
                updateButtonStates();
            };
            li.appendChild(removeBtn);
            selectedFileList.appendChild(li);
        });
        addDragAndDropHandlers();
    }

    let dragSrcElement = null;

    function handleDragStart(e) {
        this.style.opacity = '0.4';
        dragSrcElement = this;
        e.dataTransfer.effectAllowed = 'move';
        e.dataTransfer.setData('text/html', this.innerHTML);
    }

    function handleDragOver(e) {
        if (e.preventDefault) e.preventDefault();
        e.dataTransfer.dropEffect = 'move';
        return false;
    }

    function handleDragEnter(e) { this.classList.add('dragging'); }
    function handleDragLeave(e) { this.classList.remove('dragging'); }

    function handleDrop(e) {
        if (e.stopPropagation) e.stopPropagation();
        if (dragSrcElement !== this) {
            const srcIndex = parseInt(dragSrcElement.dataset.index);
            const targetIndex = parseInt(this.dataset.index);
            const [draggedItem] = selectedFilesData.splice(srcIndex, 1);
            selectedFilesData.splice(targetIndex, 0, draggedItem);
            renderFileList();
        }
        this.classList.remove('dragging');
        return false;
    }

    function handleDragEnd() {
        this.style.opacity = '1';
        document.querySelectorAll('#selectedFileList li').forEach(item => item.classList.remove('dragging'));
    }

    function addDragAndDropHandlers() {
        const items = document.querySelectorAll('#selectedFileList li');
        items.forEach(item => {
            item.addEventListener('dragstart', handleDragStart, false);
            item.addEventListener('dragenter', handleDragEnter, false);
            item.addEventListener('dragover', handleDragOver, false);
            item.addEventListener('dragleave', handleDragLeave, false);
            item.addEventListener('drop', handleDrop, false);
            item.addEventListener('dragend', handleDragEnd, false);
        });
    }

    // --- Clear Files ---
    clearFilesButton.addEventListener('click', () => {
        selectedFilesData = [];
        renderFileList();
        updateButtonStates();
        logOutput.innerHTML = '';
        progressBar.style.display = 'none';
        progressBar.value = 0;
        downloadSection.style.display = 'none';
        mergedVideoPreview.style.display = 'none';
        mergedVideoPreview.src = '';
        if (downloadLink.href) {
            URL.revokeObjectURL(downloadLink.href);
            downloadLink.removeAttribute('href');
        }
    });

    // --- Merge Button Logic ---
    mergeButton.addEventListener('click', async () => {
        if (selectedFilesData.length < 2) {
            alert('Please select at least two video files to merge.');
            return;
        }

        log('Starting merge process...');
        setProcessingState(true);

        try {
            let fileListContent = '';
            const inputFilenames = [];

            for (let i = 0; i < selectedFilesData.length; i++) {
                const file = selectedFilesData[i];
                const uniqueFileName = `input${i}.${getSafeExtension(file.name)}`;
                log(`Writing ${file.name} to FFmpeg's virtual FS as ${uniqueFileName}...`);
                ffmpeg.FS('writeFile', uniqueFileName, await fetchFile(file));
                fileListContent += `file '${uniqueFileName}'\n`;
                inputFilenames.push(uniqueFileName);
            }
            ffmpeg.FS('writeFile', 'mylist.txt', fileListContent);
            log('Generated input file list (mylist.txt).');

            const outputFilename = 'merged_output.mp4';
            log(`Running FFmpeg command: ffmpeg -f concat -safe 0 -i mylist.txt -c copy -fflags +igndts -ignore_unknown ${outputFilename}`);

            await ffmpeg.run(
                '-f', 'concat',
                '-safe', '0',
                '-i', 'mylist.txt',
                '-c', 'copy',
                '-fflags', '+igndts',
                '-ignore_unknown',
                outputFilename
            );
            log('FFmpeg processing finished.');

            const data = ffmpeg.FS('readFile', outputFilename);
            log(`Output file size: ${(data.buffer.byteLength / 1024 / 1024).toFixed(2)} MB`);

            const videoBlob = new Blob([data.buffer], { type: 'video/mp4' });
            const videoUrl = URL.createObjectURL(videoBlob);

            downloadLink.href = videoUrl;
            downloadLink.download = 'merged_video.mp4';
            mergedVideoPreview.src = videoUrl;
            mergedVideoPreview.style.display = 'block';
            downloadSection.style.display = 'block';
            log('Merged video is ready for download and preview.');

            ffmpeg.FS('unlink', 'mylist.txt');
            inputFilenames.forEach(name => ffmpeg.FS('unlink', name));
            ffmpeg.FS('unlink', outputFilename);
            log('Cleaned up virtual filesystem.');

        } catch (error) {
            log(`Error during merge: ${error.message || error}`);
            console.error("Merge Error:", error);
            alert(`An error occurred during merging: ${error.message || error}. Check console for details. This often means videos are not compatible for direct stream copy (e.g., different codecs, resolutions, or corrupted files).`);
            progressBar.value = 0; // Reset progress on error
        } finally {
            setProcessingState(false);
            // Ensure progress bar shows completion if successful, or final state on error
            if (progressBar.value > 0 && progressBar.value < 100 && !error) { // If no error but ratio wasn't 1
                progressBar.value = 100;
            }
        }
    });

    // --- Helper Functions ---
    function log(message) {
        const timestamp = new Date().toLocaleTimeString();
        logOutput.innerHTML += `[${timestamp}] ${message}\n`;
        logOutput.scrollTop = logOutput.scrollHeight;
        console.log(message);
    }

    function getSafeExtension(filename) {
        const parts = filename.split('.');
        if (parts.length > 1) {
            let ext = parts.pop().toLowerCase();
            return ext.replace(/[^a-z0-9]/gi, '') || 'mp4'; // Ensure it's sanitized or default
        }
        return 'mp4';
    }

    function setProcessingState(isProcessing) {
        mergeButton.disabled = isProcessing;
        clearFilesButton.disabled = isProcessing;
        videoFilesInput.disabled = isProcessing;
        if (isProcessing) {
            progressBar.style.display = 'block';
            progressBar.value = 0;
            downloadSection.style.display = 'none';
            mergedVideoPreview.style.display = 'none';
            mergedVideoPreview.src = '';
            if (downloadLink.href) {
                URL.revokeObjectURL(downloadLink.href);
            }
        }
    }

    function updateButtonStates() {
        const hasFiles = selectedFilesData.length > 0;
        const enoughFilesToMerge = selectedFilesData.length >= 2;
        const ffmpegReady = ffmpeg && ffmpeg.isLoaded();

        clearFilesButton.disabled = !hasFiles || mergeButton.disabled; // Also disable if processing
        mergeButton.disabled = !(ffmpegReady && enoughFilesToMerge) || videoFilesInput.disabled; // Also disable if processing

        if (!ffmpegReady) {
            mergeButton.disabled = true;
        }
    }

    // Initial state
    updateButtonStates();
});
