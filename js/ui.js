// --- UI Manipulation Functions ---

/**
 * Selects an element from the DOM.
 * @param {string} selector - The CSS selector.
 * @returns {HTMLElement|null} The selected element or null.
 */
const $ = (selector) => document.querySelector(selector);

/**
 * Selects multiple elements from the DOM.
 * @param {string} selector - The CSS selector.
 * @returns {NodeListOf<HTMLElement>} A NodeList of elements.
 */
const $$ = (selector) => document.querySelectorAll(selector);

/**
 * Renders the main header.
 * @param {object | null} user - User object (from GitHub auth) or null if not logged in.
 */
function renderHeader(user) {
  const header = $("#main-header");
  if (!header) return;

  // Determine cart item count (will get from localStorage later)
  const cartItemCount = getCartItemCount(); // Assuming this function exists in cart logic (e.g., in app.js or cart.js)

  let headerContent = `
    <a href="/" class="flex items-center gap-1">
        <img src="/logo.svg" alt="Veducation Logo" class="h-8 w-auto"> <!- Placeholder logo -->
        <span class="font-bold text-xl text-primary">Veducation</span>
    </a>
    <div class="flex items-center gap-4 relative">
  `;

  if (user) {
    // Logged in state
    headerContent += `
        <a href="/notifications.html" class="relative">
             <i class="fas fa-bell text-xl text-gray-600 hover:text-primary"></i>
             <!-- <span class="absolute top-[-5px] right-[-8px] text-xs bg-notification text-white h-4 w-4 p-1 rounded-full flex justify-center items-center border-2 border-background">3</span> --> <!-- Example notification badge -->
        </a>
        <a href="/cart.html" class="relative">
             <i class="fas fa-shopping-cart text-xl text-gray-600 hover:text-primary"></i>
             ${cartItemCount > 0 ? `<span id="cart-count-badge" class="absolute top-[-5px] right-[-8px] text-xs bg-notification text-white h-4 w-4 p-1 rounded-full flex justify-center items-center border-2 border-background">${cartItemCount}</span>` : ''}
        </a>
        <a href="/profile.html" class="relative">
             <img src="${user.avatar_url || '/icons/profile.svg'}" alt="${user.login || 'Profile'}" class="h-8 w-8 rounded-full border border-gray-300 hover:opacity-80">
        </a>
        <button id="logout-button" class="text-sm text-gray-500 hover:text-red-600">Logout</button>
    `;
  } else {
    // Logged out state
    headerContent += `
        <button id="login-button" class="bg-primary text-white text-sm font-bold py-2 px-4 rounded hover:bg-primary-dark">
            <i class="fab fa-github mr-2"></i> Login with GitHub
        </button>
    `;
  }

  headerContent += `</div>`;
  header.innerHTML = headerContent;

  // Add event listeners after rendering
  if (user) {
      const logoutButton = $('#logout-button');
      if (logoutButton) {
          logoutButton.addEventListener('click', handleLogout); // handleLogout needs to be defined in auth.js
      }
  } else {
      const loginButton = $('#login-button');
      if (loginButton) {
           loginButton.addEventListener('click', handleLogin); // handleLogin needs to be defined in auth.js
      }
  }
}

/**
 * Renders the main bottom navigation bar.
 * @param {string} activePath - The current page's path (e.g., '/', '/cart.html').
 */
function renderNavbar(activePath) {
  const navbar = $("#main-navbar");
  if (!navbar) return;

  const navItems = [
    { id: 1, iconClass: "fas fa-home", desc: "Home", href: "/" },
    { id: 2, iconClass: "fas fa-book-open", desc: "Library", href: "/library.html" }, // Assuming library.html
    { id: 3, iconClass: "fas fa-graduation-cap", desc: "Courses", href: "/courses.html" }, // Assuming courses.html
    { id: 4, iconClass: "fas fa-store", desc: "Yoddha Store", href: "/yoddha-store.html" }, // Assuming yoddha-store.html
    { id: 5, iconClass: "fas fa-bars", desc: "More", href: "/more.html" }, // Assuming more.html
  ];

  let navbarContent = '<nav class="flex justify-evenly pt-2 pb-1">'; // Reduced padding

  navItems.forEach(item => {
    // Normalize paths for comparison (remove trailing slash if present, unless it's the root)
    const normalizedActivePath = (activePath !== '/' && activePath.endsWith('/')) ? activePath.slice(0, -1) : activePath;
    const normalizedItemHref = (item.href !== '/' && item.href.endsWith('/')) ? item.href.slice(0, -1) : item.href;
    const isActive = normalizedActivePath === normalizedItemHref || (normalizedActivePath === '/index.html' && normalizedItemHref === '/');

    navbarContent += `
      <a href="${item.href}" key="${item.id}" class="flex flex-1 flex-col items-center justify-center text-center w-1/5 py-1">
        <div class="w-5 h-5 mb-1 flex items-center justify-center"> <!- Fixed size icon container -->
          <i class="${item.iconClass} text-lg ${isActive ? 'text-primary' : 'text-neutral-500'}"></i>
        </div>
        <div class="text-xs ${isActive ? 'text-primary font-semibold' : 'text-neutral-500'}">
          ${item.desc}
        </div>
      </a>
    `;
  });

  navbarContent += '</nav>';
  navbar.innerHTML = navbarContent;
}

/**
 * Renders a list of products (e.g., Books, Store Items).
 * Uses Tailwind classes directly for styling.
 * @param {string} containerSelector - CSS selector for the container element.
 * @param {string} title - The title for the product section.
 * @param {string} viewAllLink - The URL for the "View All" link.
 * @param {Array<object>} products - Array of product objects. Expected structure: { id, attributes: { title, author, posterImageUrl: { data: { attributes: { url } } } } }
 */
function renderProductList(containerSelector, title, viewAllLink, products) {
    const container = $(containerSelector);
    if (!container || !products || !products.length) {
        if(container) container.innerHTML = `<p>No products found.</p>`;
        return;
    }

    let productListHTML = `
        <div class="flex justify-between items-center mb-3">
            <h3 class="text-xl font-bold text-neutral-700 sm:text-xxl md:text-2xl mb-0">${title}</h3>
            ${viewAllLink ? `<a href="${viewAllLink}" class="text-sm text-primary hover:underline">View all</a>` : ''}
        </div>
        <div class="flex overflow-x-auto no-scrollbar gap-2 pb-2">
    `;

    products.forEach(product => {
        const imageUrl = product.attributes?.posterImageUrl?.data?.attributes?.url || '/placeholder-image.png'; // Provide a fallback
        const productTitle = product.attributes?.title || 'Untitled Product';
        const author = product.attributes?.author ? `by ${product.attributes.author}` : '';
        // Add review rendering logic here if needed

        productListHTML += `
            <a href="/product.html?id=${product.id}" class="flex-shrink-0 w-[150px] md:w-[180px]">
                <div class="flex flex-col h-full bg-transparent justify-between p-2 border border-neutral-250 rounded-md shadow-sm hover:shadow-md transition-shadow">
                    <div class="relative aspect-[2/3] w-full mb-2"> <!-- Adjusted aspect ratio for books -->
                        <img src="${imageUrl}" alt="${productTitle}" class="absolute inset-0 w-full h-full object-contain rounded-sm">
                    </div>
                    <div class="flex-grow flex flex-col justify-end">
                        <p class="text-neutral-700 text-sm font-medium leading-4 m-0 pb-1 line-clamp-2">${productTitle}</p>
                        ${author ? `<p class="text-xs font-medium leading-[9.1px] text-neutral-500">${author}</p>` : ''}
                        <!-- Reviews Placeholder -->
                        <div class="flex items-center text-xs mt-1 text-amber-500">
                           <i class="fas fa-star"></i>
                           <span class="ml-1 text-neutral-500">(Reviews)</span>
                        </div>
                    </div>
                </div>
            </a>
        `;
    });

    productListHTML += `</div>`;
    container.innerHTML = productListHTML;
}

/**
 * Renders a single component's HTML into a target container.
 * @param {string} containerSelector - The CSS selector for the container element.
 * @param {string} htmlContent - The HTML string to render.
 */
function renderComponent(containerSelector, htmlContent) {
    const container = $(containerSelector);
    if (container) {
        container.innerHTML = htmlContent;
    } else {
        console.warn(`Container element "${containerSelector}" not found.`);
    }
}

// --- Add more UI rendering functions as needed ---
// renderHomeBanner, renderAboutSection, renderCourseList, etc.
// renderCartItems, renderProductDetails, etc.

// --- Helper function (example) ---
function getCartItemCount() {
    // This should interact with the cart state (likely localStorage)
    const cart = JSON.parse(localStorage.getItem('veducationCart') || '[]');
    // Sum quantities if your cart stores items individually with quantity > 1
    // return cart.reduce((sum, item) => sum + (item.quantity || 1), 0);
    return cart.length; // Simpler if each cart entry is one item instance
}

// Function to update cart count badge specifically
function updateCartBadge() {
    const badge = $('#cart-count-badge');
    const count = getCartItemCount();
    if (badge) {
        if (count > 0) {
            badge.textContent = count;
            badge.style.display = 'flex'; // Make sure it's visible
        } else {
            badge.style.display = 'none'; // Hide if count is 0
        }
    } else if (count > 0) {
        // If badge didn't exist but should, re-render header might be needed,
        // or dynamically create and append the badge element.
        // For simplicity, ensure header re-renders on cart changes for now.
        const user = getCurrentUser(); // Assumes function exists in auth.js
         renderHeader(user);
    }
}
