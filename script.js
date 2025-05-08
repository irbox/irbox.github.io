const { createFFmpeg, fetchFile } = FFmpeg;
let ffmpeg;
let selectedFilesData = []; // To store File objects in order

document.addEventListener('DOMContentLoaded', async () => {
    const videoFilesInput = document.getElementById('videoFiles');
    const selectedFileList = document.getElementById('selectedFileList');
    const mergeButton = document.getElementById('mergeButton');
    const clearFilesButton = document.getElementById('clearFiles');
    const progressBar = document.getElementById('progressBar');
    const logOutput = document.getElementById('logOutput');
    const downloadLink = document.getElementById('downloadLink');
    const downloadSection = document.querySelector('.download-section');

    // --- Initialize FFmpeg ---
    log('Loading FFmpeg...');
    mergeButton.disabled = true;
    clearFilesButton.disabled = true;
    try {
        ffmpeg = createFFmpeg({
            // corePath: '/path/to/ffmpeg-core.js', // If self-hosting
            corePath: 'https://unpkg.com/@ffmpeg/core@0.10.0/dist/ffmpeg-core.js',
            log: true, // Enable FFmpeg's own logging
        });
        await ffmpeg.load();
        log('FFmpeg loaded successfully.');
        updateButtonStates();
    } catch (error) {
        log(`Error loading FFmpeg: ${error.message}`);
        console.error("FFmpeg loading error:", error);
        alert("Failed to load FFmpeg. Please check your internet connection or browser console for errors.");
        return;
    }


    ffmpeg.setLogger(({ type, message }) => {
        // Log FFmpeg's internal messages (optional but good for debugging)
        if (type !== 'info' && type !== 'fferr') { // Avoid too much noise
             log(`FFmpeg: ${message}`);
        }
    });

    ffmpeg.setProgress(({ ratio }) => {
        progressBar.style.display = 'block';
        progressBar.value = Math.max(0, Math.min(100, ratio * 100)); // Ensure value is between 0 and 100
        log(`Progress: ${(ratio * 100).toFixed(2)}%`);
    });

    // --- File Input Handling ---
    videoFilesInput.addEventListener('change', (event) => {
        const newFiles = Array.from(event.target.files);
        newFiles.forEach(file => {
            if (!selectedFilesData.find(f => f.name === file.name && f.size === file.size)) { // Avoid duplicates
                selectedFilesData.push(file);
            }
        });
        renderFileList();
        updateButtonStates();
        videoFilesInput.value = ''; // Reset input to allow selecting the same file again if removed
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
                e.stopPropagation(); // Prevent drag start
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
        e.dataTransfer.setData('text/html', this.innerHTML); // For Firefox
    }

    function handleDragOver(e) {
        if (e.preventDefault) {
            e.preventDefault();
        }
        e.dataTransfer.dropEffect = 'move';
        return false;
    }

    function handleDragEnter(e) {
        this.classList.add('dragging');
    }

    function handleDragLeave(e) {
        this.classList.remove('dragging');
    }

    function handleDrop(e) {
        if (e.stopPropagation) {
            e.stopPropagation();
        }
        if (dragSrcElement !== this) {
            const srcIndex = parseInt(dragSrcElement.dataset.index);
            const targetIndex = parseInt(this.dataset.index);

            // Reorder selectedFilesData array
            const [draggedItem] = selectedFilesData.splice(srcIndex, 1);
            selectedFilesData.splice(targetIndex, 0, draggedItem);

            renderFileList(); // Re-render the list to reflect new order
        }
        this.classList.remove('dragging');
        return false;
    }

    function handleDragEnd(e) {
        this.style.opacity = '1';
        const items = document.querySelectorAll('#selectedFileList li');
        items.forEach(item => {
            item.classList.remove('dragging');
        });
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
        mergeButton.disabled = true;
        clearFilesButton.disabled = true;
        videoFilesInput.disabled = true;
        progressBar.style.display = 'block';
        progressBar.value = 0;
        downloadSection.style.display = 'none';
        if (downloadLink.href) {
            URL.revokeObjectURL(downloadLink.href);
        }

        try {
            // 1. Create a file list for FFmpeg's concat demuxer
            let fileListContent = '';
            const inputFilenames = [];

            for (let i = 0; i < selectedFilesData.length; i++) {
                const file = selectedFilesData[i];
                const uniqueFileName = `input${i}.${getSafeExtension(file.name)}`;
                log(`Writing ${file.name} to FFmpeg's virtual filesystem as ${uniqueFileName}...`);
                // `fetchFile` converts the File object into a Uint8Array
                ffmpeg.FS('writeFile', uniqueFileName, await fetchFile(file));
                fileListContent += `file '${uniqueFileName}'\n`;
                inputFilenames.push(uniqueFileName);
            }
            ffmpeg.FS('writeFile', 'mylist.txt', fileListContent);
            log('Generated input file list (mylist.txt).');

            // 2. Run FFmpeg command
            // Using concat demuxer: fast, but requires files to be very similar (codec, resolution, etc.)
            // `-c copy` avoids re-encoding.
            // `-safe 0` is needed for concat demuxer when using relative paths in the list.
            // `-fflags +igndts` can sometimes help with timestamp issues.
            // `-ignore_unknown` ignores unknown stream types that might cause issues.
            const outputFilename = 'merged_output.mp4';
            log('Running FFmpeg command: ffmpeg -f concat -safe 0 -i mylist.txt -c copy -fflags +igndts -ignore_unknown ' + outputFilename);

            await ffmpeg.run(
                '-f', 'concat',
                '-safe', '0',
                '-i', 'mylist.txt',
                '-c', 'copy',        // Stream copy, much faster if compatible
                // '-bsf:a', 'aac_adtstoasc', // Sometimes needed for AAC audio
                '-fflags', '+igndts', // Ignore DTS issues that can stop concat
                '-ignore_unknown',   // Ignore unknown stream types
                outputFilename
            );
            log('FFmpeg processing finished.');

            // 3. Get the output
            const data = ffmpeg.FS('readFile', outputFilename);
            log(`Output file size: ${(data.buffer.byteLength / 1024 / 1024).toFixed(2)} MB`);

            const videoBlob = new Blob([data.buffer], { type: 'video/mp4' });
            const videoUrl = URL.createObjectURL(videoBlob);

            downloadLink.href = videoUrl;
            downloadLink.download = 'merged_video.mp4'; // Suggested download name
            downloadSection.style.display = 'block';
            log('Merged video is ready for download.');

            // 4. Cleanup FFmpeg's virtual filesystem
            ffmpeg.FS('unlink', 'mylist.txt');
            inputFilenames.forEach(name => ffmpeg.FS('unlink', name));
            ffmpeg.FS('unlink', outputFilename);
            log('Cleaned up virtual filesystem.');

        } catch (error) {
            log(`Error during merge: ${error.message}`);
            console.error("Merge Error:", error);
            alert(`An error occurred during merging: ${error}. Check console for details. Often this means videos are not compatible for direct stream copy.`);
        } finally {
            updateButtonStates(); // Re-enable based on current state
            videoFilesInput.disabled = false;
            progressBar.value = 100; // Show completion, even if an error occurred after some progress
            // Don't hide progress bar on error so user sees the final state
        }
    });

    // --- Helper Functions ---
    function log(message) {
        const timestamp = new Date().toLocaleTimeString();
        logOutput.innerHTML += `[${timestamp}] ${message}\n`;
        logOutput.scrollTop = logOutput.scrollHeight; // Auto-scroll
        console.log(message);
    }

    function getSafeExtension(filename) {
        const parts = filename.split('.');
        if (parts.length > 1) {
            let ext = parts.pop().toLowerCase();
            return ext.replace(/[^a-z0-9]/gi, ''); // Sanitize extension
        }
        return 'mp4'; // Default extension
    }

    function updateButtonStates() {
        const hasFiles = selectedFilesData.length > 0;
        const enoughFilesToMerge = selectedFilesData.length >= 2;

        clearFilesButton.disabled = !hasFiles;
        mergeButton.disabled = !(ffmpeg && ffmpeg.isLoaded() && enoughFilesToMerge);

        // If FFmpeg failed to load, merge button remains disabled
        if (ffmpeg && !ffmpeg.isLoaded()) {
            mergeButton.disabled = true;
        }
    }

    // Initial state
    updateButtonStates();
});
