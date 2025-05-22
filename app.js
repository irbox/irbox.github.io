// app.js
import { addEntry, getAllEntries, clearStore } from './db.js';
import { parseAnkiDeck } from './anki-parser.js';

// --- DOM Elements ---
const setupStatus = document.getElementById("setup-status");
const ankiFileInput = document.getElementById("anki-file-input");
const processAnkiButton = document.getElementById("process-anki-button");
const clearKbButton = document.getElementById("clear-kb-button");
const kbStatusMessage = document.getElementById("kb-status-message");
const chatSection = document.getElementById("chat-section");
const chatContainer = document.getElementById("chat-container");
const userInput = document.getElementById("user-input");
const sendButton = document.getElementById("send-button");
const modelStatusDisplay = document.getElementById("model-status");

// --- Global Variables ---
let model; // The Universal Sentence Encoder model
let knowledgeBase = []; // Stores objects with {question, answer, embedding}
let isLoading = true; // Flag to indicate if model is loading or processing

// --- Configuration ---
const SIMILARITY_THRESHOLD = 0.7; // Adjust for stricter/looser matching

// --- Utility Function: Cosine Similarity ---
function cosineSimilarity(vec1, vec2) {
    let dotProduct = 0;
    let magnitude1 = 0;
    let magnitude2 = 0;
    for (let i = 0; i < vec1.length; i++) {
        dotProduct += vec1[i] * vec2[i];
        magnitude1 += vec1[i] * vec1[i];
        magnitude2 += vec2[i] * vec2[i];
    }
    magnitude1 = Math.sqrt(magnitude1);
    magnitude2 = Math.sqrt(magnitude2);
    if (magnitude1 === 0 || magnitude2 === 0) return 0;
    return dotProduct / (magnitude1 * magnitude2);
}

// --- Functions ---

/**
 * Adds a message to the chat container.
 * @param {string} content - The message content (can be HTML).
 * @param {string} sender - 'user' or 'ai' or 'system'.
 */
function addMessage(content, sender) {
    const messageDiv = document.createElement("div");
    messageDiv.classList.add("message", `${sender}-message`);
    if (sender === 'ai') { // Allow HTML for AI responses from Anki
        messageDiv.innerHTML = content;
    } else {
        messageDiv.textContent = content;
    }
    chatContainer.appendChild(messageDiv);
    chatContainer.scrollTop = chatContainer.scrollHeight;
}

/**
 * Updates UI based on knowledge base status.
 */
function updateKbStatusUI() {
    const kbLoaded = knowledgeBase.length > 0;
    if (kbLoaded) {
        kbStatusMessage.textContent = `Knowledge base loaded: ${knowledgeBase.length} Q&A pairs.`;
        kbStatusMessage.style.backgroundColor = '#d4edda'; // Greenish for success
        kbStatusMessage.style.color = '#155724';
        chatSection.style.display = 'flex'; // Show chat section
        ankiFileInput.disabled = true;
        processAnkiButton.disabled = true;
        clearKbButton.disabled = false;
        userInput.disabled = false;
        sendButton.disabled = false;
    } else {
        kbStatusMessage.textContent = "No knowledge base loaded. Please upload an Anki deck.";
        kbStatusMessage.style.backgroundColor = '#ffeeba'; // Yellowish for warning
        kbStatusMessage.style.color = '#856404';
        chatSection.style.display = 'none'; // Hide chat section
        ankiFileInput.disabled = false;
        processAnkiButton.disabled = true; // Still disabled until file selected
        clearKbButton.disabled = true;
        userInput.disabled = true;
        sendButton.disabled = true;
    }
}


/**
 * Handles the sending of a user message.
 */
async function sendMessage() {
    const prompt = userInput.value.trim();
    if (!prompt || isLoading || knowledgeBase.length === 0) {
        return;
    }

    isLoading = true;
    userInput.disabled = true;
    sendButton.disabled = true;
    setupStatus.textContent = "AI is processing...";
    setupStatus.style.display = 'block';

    addMessage(prompt, "user");
    userInput.value = ""; // Clear input

    try {
        // Embed the user's query
        const userEmbeddingTensor = await model.embed([prompt]);
        const userEmbedding = userEmbeddingTensor.arraySync()[0];
        userEmbeddingTensor.dispose(); // Clean up tensor memory

        let bestMatch = null;
        let highestSimilarity = -1;

        // Find the most similar question in the knowledge base
        for (const item of knowledgeBase) {
            const similarity = cosineSimilarity(userEmbedding, item.embedding);
            if (similarity > highestSimilarity) {
                highestSimilarity = similarity;
                bestMatch = item;
            }
        }

        let aiResponse;
        if (bestMatch && highestSimilarity >= SIMILARITY_THRESHOLD) {
            aiResponse = bestMatch.answer;
        } else {
            aiResponse = "I'm sorry, I don't understand that question from my knowledge base. Please try rephrasing.";
        }

        addMessage(aiResponse, "ai");
        setupStatus.style.display = 'none';

    } catch (error) {
        console.error("Error processing message:", error);
        addMessage("Error: Something went wrong while processing your request.", "ai");
        setupStatus.textContent = `Error: ${error.message}`;
        setupStatus.style.display = 'block';
        setupStatus.style.color = 'red';
    } finally {
        isLoading = false;
        userInput.disabled = false;
        sendButton.disabled = false;
        userInput.focus();
        setupStatus.style.color = '#333'; // Reset color
    }
}

/**
 * Processes the uploaded Anki deck.
 */
async function processAnkiDeckFile() {
    const file = ankiFileInput.files[0];
    if (!file) {
        setupStatus.textContent = "Please select an Anki deck file.";
        setupStatus.style.color = 'orange';
        setupStatus.style.display = 'block';
        return;
    }

    isLoading = true;
    ankiFileInput.disabled = true;
    processAnkiButton.disabled = true;
    clearKbButton.disabled = true;
    userInput.disabled = true;
    sendButton.disabled = true;
    setupStatus.textContent = `Processing "${file.name}"... This may take a while for large decks.`;
    setupStatus.style.color = '#333';
    setupStatus.style.display = 'block';

    try {
        const arrayBuffer = await file.arrayBuffer();
        const qaPairs = await parseAnkiDeck(arrayBuffer);

        if (qaPairs.length === 0) {
            throw new Error('No Q&A pairs could be extracted from the Anki deck. Ensure it has notes with at least two text fields.');
        }

        // Clear existing KB in IndexedDB
        await clearStore();
        knowledgeBase = []; // Reset in memory

        // Embed and store
        const questionsToEmbed = qaPairs.map(item => item.question);
        setupStatus.textContent = `Embedding ${questionsToEmbed.length} questions...`;
        const embeddingsTensor = await model.embed(questionsToEmbed);
        const embeddings = embeddingsTensor.arraySync();
        embeddingsTensor.dispose(); // Clean up tensor memory

        for (let i = 0; i < qaPairs.length; i++) {
            const entry = {
                question: qaPairs[i].question,
                answer: qaPairs[i].answer, // Store original Anki HTML
                embedding: embeddings[i]
            };
            await addEntry(entry);
            knowledgeBase.push(entry);
        }

        setupStatus.textContent = `Successfully loaded ${knowledgeBase.length} Q&A pairs from Anki deck.`;
        setupStatus.style.backgroundColor = '#d4edda';
        setupStatus.style.color = '#155724';

    } catch (error) {
        console.error("Error processing Anki deck:", error);
        setupStatus.textContent = `Error: ${error.message}. Please try another file.`;
        setupStatus.style.backgroundColor = '#f8d7da';
        setupStatus.style.color = '#721c24';
    } finally {
        isLoading = false;
        ankiFileInput.value = ''; // Clear file input for next upload
        ankiFileInput.disabled = false;
        updateKbStatusUI(); // Enable buttons based on KB status
        userInput.focus();
    }
}

/**
 * Clears the knowledge base.
 */
async function clearKnowledgeBase() {
    if (!confirm("Are you sure you want to clear the entire knowledge base? This action cannot be undone.")) {
        return;
    }

    isLoading = true;
    ankiFileInput.disabled = true;
    processAnkiButton.disabled = true;
    clearKbButton.disabled = true;
    userInput.disabled = true;
    sendButton.disabled = true;
    setupStatus.textContent = "Clearing knowledge base...";
    setupStatus.style.color = '#333';
    setupStatus.style.display = 'block';

    try {
        await clearStore();
        knowledgeBase = [];
        setupStatus.textContent = "Knowledge base cleared successfully.";
        setupStatus.style.backgroundColor = '#ffeeba';
        setupStatus.style.color = '#856404';
    } catch (error) {
        console.error("Error clearing knowledge base:", error);
        setupStatus.textContent = `Error clearing KB: ${error.message}`;
        setupStatus.style.backgroundColor = '#f8d7da';
        setupStatus.style.color = '#721c24';
    } finally {
        isLoading = false;
        updateKbStatusUI();
    }
}


/**
 * Initializes the Universal Sentence Encoder model and loads existing KB from IndexedDB.
 */
async function initApp() {
    isLoading = true;
    ankiFileInput.disabled = true;
    processAnkiButton.disabled = true;
    clearKbButton.disabled = true;
    userInput.disabled = true;
    sendButton.disabled = true;
    setupStatus.style.display = 'block';
    setupStatus.textContent = "Initializing app...";
    modelStatusDisplay.textContent = 'Loading model...';

    try {
        // Load the Universal Sentence Encoder model
        model = await use.load();
        modelStatusDisplay.textContent = 'Model loaded!';
        setupStatus.textContent = "AI model loaded. Checking for existing knowledge base...";

        // Load knowledge base from IndexedDB
        const storedKb = await getAllEntries();
        if (storedKb.length > 0) {
            knowledgeBase = storedKb;
            setupStatus.textContent = `Loaded ${knowledgeBase.length} Q&A pairs from local storage.`;
            setupStatus.style.backgroundColor = '#d4edda';
            setupStatus.style.color = '#155724';
        } else {
            setupStatus.textContent = "No knowledge base found. Please upload an Anki deck to start.";
            setupStatus.style.backgroundColor = '#ffeeba';
            setupStatus.style.color = '#856404';
        }

    } catch (error) {
        console.error("Failed to load AI model or knowledge base:", error);
        modelStatusDisplay.textContent = 'Error loading model!';
        setupStatus.textContent = `Fatal Error: ${error.message}. Please ensure all files are served correctly and your browser is compatible.`;
        setupStatus.style.backgroundColor = '#f8d7da';
        setupStatus.style.color = '#721c24';
    } finally {
        isLoading = false;
        updateKbStatusUI(); // Enable buttons based on KB status
        userInput.focus();
    }
}

// --- Event Listeners ---
ankiFileInput.addEventListener("change", () => {
    processAnkiButton.disabled = ankiFileInput.files.length === 0;
});
processAnkiButton.addEventListener("click", processAnkiDeckFile);
clearKbButton.addEventListener("click", clearKnowledgeBase);
sendButton.addEventListener("click", sendMessage);
userInput.addEventListener("keypress", (event) => {
    if (event.key === "Enter" && !sendButton.disabled) {
        sendMessage();
    }
});

// --- Initialize the app when the page loads ---
document.addEventListener("DOMContentLoaded", initApp);
