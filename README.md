```
/ (repository root)
├── index.html
├── product.html
├── cart.html
├── library.html
├── courses.html
├── yoddha-store.html
├── profile.html
├── more.html
├── billingdetails.html
├── orderconfirmed.html
├── about.html          (If you create a separate page)
├── donations.html      (If you create a separate page)
├── products.html       (If you create a separate page)
├── css/
│   └── custom.css
├── js/
│   ├── app.js
│   ├── api.js
│   ├── auth.js
│   ├── ui.js
│   ├── utils.js
│   ├── billing.js
│   └── orderconfirmed.js
├── images/             (Or place directly in root, adjust paths accordingly)
│   ├── AboutPageImg.png
│   ├── book_sanatan_sanskriti.png
│   ├── book_basics_sanatan.png
│   ├── book_new_yorkers.jpg
│   ├── book_javascript.jpg
│   ├── tshirt_black.png
│   ├── store_mugdar.png
│   ├── store_mala.png
│   ├── course_soul.png
│   ├── course_placeholder_2.png
│   ├── img1.png
│   ├── img2.png
│   ├── img4.png
│   └── placeholder-image.png (Create this)
├── icons/
│   └── profile.svg      (Placeholder if GitHub avatar fails)
│   └── (Potentially others if not using Font Awesome)
├── videos/
│   └── sample_video.mp4 (Or other video files)
├── logo.svg            (In root or /images/)
└── favicon.ico         (In root)
```


## Technologies Used

*   **HTML5:** Semantic structure for web pages.
*   **CSS3:** Styling, using Tailwind CSS via CDN for utility classes and `custom.css` for specifics.
*   **JavaScript (ES6+):** Client-side interactivity, DOM manipulation, simulated state management (`localStorage`), and simulated API calls.
*   **Tailwind CSS (CDN):** Utility-first CSS framework for rapid styling.
*   **Font Awesome (CDN):** For icons (can be replaced if needed).

## Getting Started (Local Development)

1.  **Clone or Download:** Get the project files onto your local machine.
2.  **Open `index.html`:** Simply open the `index.html` file directly in your web browser (e.g., Chrome, Firefox, Edge).
3.  **Navigate:** Click on the links to navigate between the different HTML pages.

No build step or local server is strictly required for basic viewing due to the use of CDNs and the static nature.

## Deployment (GitHub Pages)

This project is structured for easy deployment using GitHub Pages:

1.  **Create GitHub Repository:** Create a new repository on GitHub.com (e.g., `veducation-static`).
2.  **Upload Files:** Upload all the files and folders (`index.html`, `css/`, `js/`, `images/`, etc.) directly to the root of your repository using the GitHub web interface ("Add file" -> "Upload files").
3.  **Enable GitHub Pages:**
    *   Go to your repository's **Settings** tab.
    *   Navigate to the **Pages** section in the left sidebar.
    *   Under "Build and deployment", select **Deploy from a branch**.
    *   Choose the branch you uploaded the files to (usually `main`).
    *   Select the `/ (root)` folder.
    *   Click **Save**.
4.  **Access Site:** GitHub Pages will build and deploy your site. It might take a minute or two. The URL will be displayed on the Pages settings screen (usually `https://<your-username>.github.io/<repository-name>/`).

## Limitations

*   **Simulated Backend:** All dynamic actions (login, saving data, checkout) are simulated using `localStorage` or simple JS logic. There is no real backend database or server-side processing.
*   **Security:** GitHub OAuth cannot be securely implemented without a backend. The current login is purely for demonstration purposes within the browser session.
*   **Data:** All product, course, and other content data is currently hardcoded within `js/api.js`. It needs to be manually updated in that file.

## Contributing

This project is a static reconstruction. Contributions for fixing bugs or improving the static implementation are welcome. Please open an issue or submit a pull request.
