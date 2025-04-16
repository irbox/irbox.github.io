// --- Authentication Functions (Simulated GitHub Login) ---

const GITHUB_USER_KEY = 'veducationGithubUser';

/**
 * Simulates initiating the GitHub login flow.
 * In a real app, this would redirect to GitHub's authorization URL.
 * Here, we'll just simulate a successful login immediately for demo purposes.
 */
function handleLogin() {
  console.log("Simulating GitHub Login...");

  // --- Real GitHub OAuth would involve redirecting: ---
  // const GITHUB_CLIENT_ID = 'YOUR_GITHUB_CLIENT_ID'; // Replace with your actual Client ID
  // const redirectUri = window.location.origin + '/oauth_callback.html'; // A dedicated callback page
  // const scope = 'read:user user:email'; // Define requested permissions
  // const authUrl = `https://github.com/login/oauth/authorize?client_id=${GITHUB_CLIENT_ID}&redirect_uri=${encodeURIComponent(redirectUri)}&scope=${encodeURIComponent(scope)}&response_type=code`;
  // window.location.href = authUrl;
  // -----------------------------------------------------

  // --- Simulation Start ---
  // Create a dummy user object
  const dummyUser = {
    login: 'static-user',
    id: 12345,
    avatar_url: 'https://avatars.githubusercontent.com/u/12345?v=4', // Example avatar
    name: 'Static User',
    email: 'static-user@example.com' // Example email
    // Add other relevant public fields if needed
  };

  // Store the dummy user in localStorage
  localStorage.setItem(GITHUB_USER_KEY, JSON.stringify(dummyUser));
  console.log("Simulated login successful. User data stored.");

  // Update UI immediately (re-render header, potentially redirect or update page content)
  renderHeader(dummyUser); // Assumes renderHeader is globally available from ui.js
  // Optionally redirect to the main page or profile page
  // window.location.href = '/'; // Or '/profile.html'
  // Or trigger a content update function from app.js
  if (typeof initializePage === 'function') {
      initializePage(); // Re-initialize page content based on new auth state
  }
  // --- Simulation End ---
}

/**
 * Handles the OAuth callback.
 * In a real app, this page/function would:
 * 1. Get the 'code' from the URL query parameters.
 * 2. Send the 'code' to a backend/serverless function.
 * 3. Backend exchanges code + client secret for an access token.
 * 4. Backend fetches user details from GitHub API using the token.
 * 5. Backend sends user details back to this callback page.
 * 6. This callback page stores the user details and redirects.
 *
 * Since we can't do steps 2-5 securely on the client-side, this is simplified/simulated.
 */
function handleOAuthCallback() {
    console.log("Handling OAuth Callback (Simulated)...");
    // --- Simulation: Assume login was successful (handled in handleLogin directly for static demo) ---
    // In a real scenario with a backend, you'd parse the user data received
    // from your backend here and store it before redirecting.

    // Redirect back to the main page after storing data (if needed)
    // window.location.href = '/';
}


/**
 * Handles logging the user out.
 */
function handleLogout() {
  console.log("Logging out...");
  localStorage.removeItem(GITHUB_USER_KEY);
  console.log("User data removed.");

  // Update UI (re-render header, potentially redirect or update page content)
  renderHeader(null); // Pass null to indicate logged-out state
  // Optionally redirect to home page
  // window.location.href = '/';
  // Or trigger a content update function from app.js
  if (typeof initializePage === 'function') {
      initializePage(); // Re-initialize page content based on new auth state
  }
}

/**
 * Checks if a user is currently logged in (based on localStorage).
 * @returns {boolean} True if user data exists, false otherwise.
 */
function isLoggedIn() {
  return !!localStorage.getItem(GITHUB_USER_KEY);
}

/**
 * Retrieves the current user's data from localStorage.
 * @returns {object | null} The user object or null if not logged in.
 */
function getCurrentUser() {
  const userJson = localStorage.getItem(GITHUB_USER_KEY);
  try {
    return userJson ? JSON.parse(userJson) : null;
  } catch (error) {
    console.error("Error parsing user data from localStorage:", error);
    localStorage.removeItem(GITHUB_USER_KEY); // Clear corrupted data
    return null;
  }
}

// Example of how the callback might be triggered if we had a separate callback page
// (Not used in the direct simulation within handleLogin)
// function checkOAuthCallback() {
//     const urlParams = new URLSearchParams(window.location.search);
//     const code = urlParams.get('code');
//     const error = urlParams.get('error');

//     if (error) {
//         console.error("GitHub OAuth Error:", error, urlParams.get('error_description'));
//         // Display error to user
//         // Maybe redirect to login page with error message
//     } else if (code) {
//         // In a real app, send 'code' to backend here
//         console.log("Received GitHub code:", code); // Don't log in production!
//         // For simulation, we assume the backend would succeed and return user data
//         handleOAuthCallback();
//         // Remove code from URL history for cleanliness
//         window.history.replaceState({}, document.title, window.location.pathname);
//     }
// }
