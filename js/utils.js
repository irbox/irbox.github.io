// --- Utility Functions ---

/**
 * Formats a price value to a currency string (e.g., $19.99).
 * Handles null or undefined values.
 * @param {number | string | null | undefined} price - The price value.
 * @param {string} [currency='USD'] - The currency code.
 * @param {string} [locale='en-US'] - The locale for formatting.
 * @returns {string} The formatted currency string (e.g., "$19.99") or a placeholder (e.g., "N/A").
 */
function formatPrice(price, currency = 'USD', locale = 'en-US') {
  const numericPrice = parseFloat(price);
  if (isNaN(numericPrice)) {
    return 'N/A'; // Or return '$0.00' or empty string based on preference
  }
  try {
    return new Intl.NumberFormat(locale, {
      style: 'currency',
      currency: currency,
    }).format(numericPrice);
  } catch (error) {
    console.error("Error formatting price:", error);
    // Fallback for environments without Intl support or invalid options
    return `$${numericPrice.toFixed(2)}`;
  }
}

/**
 * Truncates a string to a maximum length, adding an ellipsis if truncated.
 * @param {string} str - The string to truncate.
 * @param {number} maxLength - The maximum allowed length.
 * @returns {string} The truncated string.
 */
function truncateString(str, maxLength) {
  if (!str || typeof str !== 'string') {
    return '';
  }
  if (str.length <= maxLength) {
    return str;
  }
  return str.slice(0, maxLength) + '...';
}

/**
 * Debounces a function, ensuring it's only called after a certain delay
 * since the last time it was invoked.
 * @param {Function} func - The function to debounce.
 * @param {number} delay - The debounce delay in milliseconds.
 * @returns {Function} The debounced function.
 */
function debounce(func, delay) {
  let timeoutId;
  return function(...args) {
    clearTimeout(timeoutId);
    timeoutId = setTimeout(() => {
      func.apply(this, args);
    }, delay);
  };
}

/**
 * Gets a query parameter value from the current URL.
 * @param {string} paramName - The name of the query parameter.
 * @returns {string | null} The value of the parameter or null if not found.
 */
function getQueryParam(paramName) {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(paramName);
}

/**
// * Converts Markdown text to HTML using markdown-it library.
// * Assumes markdown-it library is loaded globally or via import if using modules.
// * @param {string} markdownText - The markdown text to convert.
// * @returns {string} The resulting HTML string.
// */
// function renderMarkdown(markdownText = '') {
//     // Ensure the library is available (you might load it via CDN in index.html)
//     if (typeof window.markdownit === 'undefined') {
//         console.error('markdown-it library is not loaded.');
//         return `<p>${markdownText}</p>`; // Fallback: return as plain text paragraph
//     }
//     const md = window.markdownit();
//     return md.render(markdownText);
// }

// Add more utility functions as needed (e.g., date formatting, slugify, etc.)
