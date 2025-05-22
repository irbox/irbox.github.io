// app.js

// --- DOM Elements ---
const setupStatus = document.getElementById("setup-status");
const apiKeyInput = document.getElementById("api-key-input");
const modelSelect = document.getElementById("model-select");
const activateAiButton = document.getElementById("activate-ai-button");
const chatSection = document.getElementById("chat-section");
const chatContainer = document.getElementById("chat-container");
const userInput = document.getElementById("user-input");
const sendButton = document.getElementById("send-button");
const modelStatusDisplay = document.getElementById("model-status");

// --- Global Variables ---
const GEMINI_API_BASE_URL = "https://generativelanguage.googleapis.com/v1beta/";
let currentApiKey = '';
let currentModelId = '';
let chatHistory = []; // Stores conversation for Gemini API
let isLoading = true; // Flag for API calls

// --- Functions ---

/**
 * Updates the main setup status message with new text and styling.
 * @param {string} text - The message content.
 * @param {string} type - 'loading', 'success', 'warning', 'error'
 * @param {boolean} showSpinner - Whether to show the spinner.
 */
function updateSetupStatus(text, type = 'loading', showSpinner = false) {
    setupStatus.innerHTML = showSpinner ? `<div class="spinner"></div> ${text}` : text;
    setupStatus.classList.remove('loading', 'success', 'warning', 'error');
    setupStatus.classList.add(type);
    setupStatus.style.display = 'block';
}

/**
 * Adds a message to the chat container.
 * @param {string} content - The message content.
 * @param {string} sender - 'user' or 'ai'.
 * @param {string} id - Optional ID for the message div (for typing indicator)
 */
function addMessage(content, sender, id = null) {
    const messageDiv = document.createElement("div");
    messageDiv.classList.add("message", `${sender}-message`);
    if (id) {
        messageDiv.id = id;
    }
    messageDiv.textContent = content;
    chatContainer.appendChild(messageDiv);
    chatContainer.scrollTop = chatContainer.scrollHeight;
}

/**
 * Updates the content of an existing message.
 * @param {string} id - The ID of the message element.
 * @param {string} newContent - The new HTML content.
 */
function updateMessageContent(id, newContent) {
    const messageDiv = document.getElementById(id);
    if (messageDiv) {
        messageDiv.textContent = newContent;
        chatContainer.scrollTop = chatContainer.scrollHeight;
    }
}

/**
 * Adds a typing indicator message.
 * @returns {string} The ID of the typing indicator message element.
 */
function addTypingIndicator() {
    const id = `ai-typing-${Date.now()}`;
    const messageDiv = document.createElement("div");
    messageDiv.classList.add("message", "ai-message", "typing-indicator");
    messageDiv.id = id;
    messageDiv.innerHTML = '<span></span><span></span><span></span>';
    chatContainer.appendChild(messageDiv);
    chatContainer.scrollTop = chatContainer.scrollHeight;
    return id;
}

/**
 * Removes the typing indicator message.
 * @param {string} id - The ID of the typing indicator message element.
 */
function removeTypingIndicator(id) {
    const indicator = document.getElementById(id);
    if (indicator) {
        indicator.remove();
    }
}

/**
 * Fetches available Gemini models and populates the dropdown.
 */
async function fetchAvailableModels() {
    modelSelect.innerHTML = '<option value="">Loading models...</option>';
    modelSelect.disabled = true;
    activateAiButton.disabled = true;
    modelStatusDisplay.textContent = 'Fetching models...';

    const apiKey = apiKeyInput.value.trim();
    if (!apiKey) {
        updateSetupStatus("Please enter your Gemini API key.", 'warning');
        return;
    }

    try {
        const response = await fetch(`${GEMINI_API_BASE_URL}models?key=${apiKey}`);
        if (!response.ok) {
            const errorData = await response.json();
            throw new Error(`API Error: ${errorData.error.message || response.statusText}`);
        }
        const data = await response.json();

        // Filter for chat-compatible models and exclude vision models
        const models = data.models.filter(m =>
            m.supportedGenerationMethods.includes('generateContent') &&
            !m.name.includes('vision') &&
            !m.name.includes('embedding') // Exclude embedding models
        );

        if (models.length === 0) {
            modelSelect.innerHTML = '<option value="">No chat models found.</option>';
            updateSetupStatus("No suitable chat models found with this API key.", 'warning');
            modelStatusDisplay.textContent = 'No models';
            return;
        }

        modelSelect.innerHTML = ''; // Clear existing options
        models.forEach(model => {
            const option = document.createElement('option');
            option.value = model.name;
            option.textContent = model.displayName || model.name;
            modelSelect.appendChild(option);
        });

        modelSelect.disabled = false;
        activateAiButton.disabled = false; // Enable activation button if models are loaded
        updateSetupStatus("Models loaded. Enter API Key and activate AI.", 'success');
        modelStatusDisplay.textContent = 'Models ready';

    } catch (error) {
        console.error("Error fetching models:", error);
        modelSelect.innerHTML = '<option value="">Failed to load models.</option>';
        updateSetupStatus(`Error fetching models: ${error.message}. Check API key.`, 'error');
        modelStatusDisplay.textContent = 'Error';
    } finally {
        isLoading = false;
    }
}

/**
 * Activates the AI chat with the selected model.
 */
function activateAI() {
    currentApiKey = apiKeyInput.value.trim();
    currentModelId = modelSelect.value;

    if (!currentApiKey) {
        updateSetupStatus("Please enter your Gemini API key.", 'error');
        return;
    }
    if (!currentModelId) {
        updateSetupStatus("Please select an AI model.", 'error');
        return;
    }

    // Hide setup, show chat
    setupStatus.style.display = 'none'; // Hide the setup status message
    document.getElementById('setup-section').style.display = 'none';
    chatSection.style.display = 'flex';
    modelStatusDisplay.textContent = `Active: ${modelSelect.options[modelSelect.selectedIndex].text}`;

    // Reset chat history when activating AI to start fresh
    chatHistory = [];
    chatContainer.innerHTML = '';
    addMessage("Hello! I'm ready to chat. How can I help you?", "ai");

    userInput.focus();
    isLoading = false; // AI is now active and ready for input
}

/**
 * Handles sending a user message to Gemini API.
 */
async function sendMessage() {
    const prompt = userInput.value.trim();
    if (!prompt || isLoading || !currentApiKey || !currentModelId) {
        return;
    }

    isLoading = true;
    userInput.disabled = true;
    sendButton.disabled = true;
    modelStatusDisplay.textContent = 'AI is thinking...';

    addMessage(prompt, "user");
    userInput.value = ""; // Clear input

    const typingIndicatorId = addTypingIndicator(); // Show typing indicator

    // Add user message to chat history
    chatHistory.push({ role: 'user', parts: [{ text: prompt }] });

    try {
        const streamResponse = await fetch(
            `${GEMINI_API_BASE_URL}models/${currentModelId}:streamGenerateContent?key=${currentApiKey}`,
            {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ contents: chatHistory }),
            }
        );

        if (!streamResponse.ok) {
            const errorData = await streamResponse.json();
            throw new Error(`API Error: ${errorData.error.message || streamResponse.statusText}`);
        }

        const reader = streamResponse.body.getReader();
        const decoder = new TextDecoder();
        let aiResponse = '';
        let firstChunk = true;

        while (true) {
            const { value, done } = await reader.read();
            if (done) break;

            const chunk = decoder.decode(value, { stream: true });
            const data = JSON.parse(chunk.split('\n')[0].replace(/^data: /, '')); // Handle potential 'data: ' prefix

            if (data.candidates && data.candidates[0].content && data.candidates[0].content.parts) {
                const newText = data.candidates[0].content.parts[0].text || '';
                aiResponse += newText;

                if (firstChunk) {
                    removeTypingIndicator(typingIndicatorId); // Remove indicator on first token
                    addMessage(aiResponse, "ai", `ai-message-${Date.now()}`); // Add initial AI message
                    firstChunk = false;
                } else {
                    updateMessageContent(chatContainer.lastChild.id, aiResponse); // Update the last AI message
                }
            }
        }
        // Add full AI response to chat history
        chatHistory.push({ role: 'model', parts: [{ text: aiResponse }] });
        modelStatusDisplay.textContent = `Active: ${modelSelect.options[modelSelect.selectedIndex].text}`;

    } catch (error) {
        console.error("Error generating content:", error);
        removeTypingIndicator(typingIndicatorId);
        addMessage(`Error: Could not get response. ${error.message}`, "ai");
        modelStatusDisplay.textContent = `Error: ${error.message.substring(0, 50)}...`; // Truncate error
    } finally {
        isLoading = false;
        userInput.disabled = false;
        sendButton.disabled = false;
        userInput.focus();
    }
}

// --- Event Listeners ---
apiKeyInput.addEventListener("input", () => {
    // Re-fetch models if API key changes
    fetchAvailableModels();
});

activateAiButton.addEventListener("click", activateAI);

sendButton.addEventListener("click", sendMessage);
userInput.addEventListener("keypress", (event) => {
    if (event.key === "Enter" && !sendButton.disabled) {
        sendMessage();
    }
});

// --- Initialize the app when the page loads ---
document.addEventListener("DOMContentLoaded", () => {
    // Try to load API key from localStorage if it exists (for convenience during dev)
    const storedApiKey = localStorage.getItem('geminiApiKey');
    if (storedApiKey) {
        apiKeyInput.value = storedApiKey;
    }
    fetchAvailableModels();
});

// Store API key in localStorage on input change for convenience
apiKeyInput.addEventListener('change', () => {
    localStorage.setItem('geminiApiKey', apiKeyInput.value.trim());
});
