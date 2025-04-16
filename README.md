# Veducation Static Website

This is a static HTML, CSS, and JavaScript version of the Veducation website, reconstructed from the original Next.js project. It is designed for easy deployment on static hosting platforms like GitHub Pages.

## Features

*   Browse Courses, Yoddha Store items, Library books, and Donations.
*   View item details.
*   Add items to a client-side shopping cart using `localStorage`.
*   Simulated checkout process.
*   Basic "Login with GitHub" integration using Client-side OAuth flow.
*   Responsive design using compiled CSS (originally Tailwind).

## Tech Stack

*   HTML5
*   CSS3 (Requires compiled output from original project's Tailwind + migrated custom styles)
*   Vanilla JavaScript (ES6+)
*   JSON (for static data storage)
*   GitHub OAuth (Client-side flow for authentication simulation/public info)

## Data Storage

*   **Content:** All dynamic content (products, courses, donations, books) should be placed in JSON files within the `/data` directory. This version includes example JSON files. You need to populate these with your actual data. Content is fetched client-side using the browser's `fetch` API.
*   **Cart:** The shopping cart state is stored in the browser's `localStorage`.
*   **Login State:** User login status and basic GitHub user info are stored in `localStorage`.

## Authentication

Authentication uses a client-side GitHub OAuth flow initiated from the `/login.html` page and handled by `/callback.html`.

**Important Security Note:** This implementation demonstrates the *client-side* part of OAuth. For simplicity in this static context, the crucial step of exchanging the received code for an access token (which requires a client secret and should **never** be done directly in the browser) is **simulated** in `auth.js`. A real-world application requires a secure backend or serverless function (like a Cloudflare Worker, Netlify Function, or Vercel Function) to handle the token exchange securely. This version primarily stores login status and public user info fetched after the simulated successful callback.

## Getting Started Locally

1.  Clone or download this repository.
2.  **Copy Assets:** Manually copy the contents of the `public/` folder from your original Next.js project into the `assets/` folder (create `assets/images/` and `assets/icons/` as needed).
3.  **Compile & Migrate CSS:**
    *   Run the build command (e.g., `npm run build` or `pnpm build`) on your *original* Next.js project.
    *   Locate the main compiled CSS file (usually in `.next/static/css/`).
    *   Copy the *entire content* of that compiled CSS file and paste it at the **top** of the `style.css` file provided in this static project.
    *   Go through the `*.module.css` files in your original project (`src/app/...` and `src/components/...`). Copy the custom CSS rules from these files into the `style.css` file (below the compiled Tailwind part). **Rename the selectors** from the module format (e.g., `.main__detail`) to standard CSS class names (e.g., `.cart-item-detail`) and ensure the JavaScript code uses these new class names when generating HTML.
4.  **Populate Data:** Edit the `.json` files in the `/data` directory with your actual product, course, book, and donation information.
5.  **Configure GitHub Auth:** Edit `auth.js` and replace `'YOUR_GITHUB_CLIENT_ID'` with your actual GitHub OAuth App Client ID. Ensure the Redirect URI in your GitHub App settings is set to `http://<your-local-server-or-gh-pages-url>/callback.html`.
6.  **Run Locally:** Open the `index.html` file in your web browser. For best results (especially `fetch` and `localStorage`), use a simple local HTTP server:
    *   If you have Python: `python -m http.server`
    *   Using Node.js: `npx serve .`
    *   Or use VS Code's "Live Server" extension.

## Deployment on GitHub Pages

1.  Ensure your code (HTML, CSS, JS, data, assets) is committed and pushed to the `main` branch (or another branch you choose) of your GitHub repository.
2.  Go to your repository's **Settings** tab on GitHub.
3.  Navigate to the **Pages** section in the left sidebar.
4.  Under "Build and deployment":
    *   Source: Select **Deploy from a branch**.
    *   Branch: Choose `main` (or your deployment branch) and `/ (root)` folder.
5.  Click **Save**.
6.  GitHub Actions will deploy your site. The URL will appear on the Pages settings screen once finished (it might take a minute or two). **Remember to update the Redirect URI in your GitHub OAuth App settings to match your live GitHub Pages URL.**
