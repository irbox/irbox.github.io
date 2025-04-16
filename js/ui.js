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

  const cartItemCount = getCartItemCount(); // Assumes exists (defined in app.js or cart.js)

  let headerContent = `
    <a href="/" class="flex items-center gap-1">
        <img src="/logo.svg" alt="Veducation Logo" class="h-8 w-auto"> <!-- Placeholder logo -->
        <span class="font-bold text-xl text-primary">Veducation</span>
    </a>
    <div class="flex items-center gap-4 relative">
  `;

  if (user) {
    // Logged in state
    headerContent += `
        <a href="/notifications.html" class="relative">
             <i class="fas fa-bell text-xl text-gray-600 hover:text-primary"></i>
             <!-- Notification badge logic can be added here -->
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
          // Remove previous listener if any to prevent duplicates
          logoutButton.removeEventListener('click', handleLogout);
          logoutButton.addEventListener('click', handleLogout); // handleLogout needs to be defined in auth.js
      }
  } else {
      const loginButton = $('#login-button');
      if (loginButton) {
           // Remove previous listener if any
           loginButton.removeEventListener('click', handleLogin);
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
    { id: 1, iconClass: "fas fa-home", desc: "Home", href: "/index.html" }, // Point explicitly to index.html
    { id: 2, iconClass: "fas fa-book-open", desc: "Library", href: "/library.html" },
    { id: 3, iconClass: "fas fa-graduation-cap", desc: "Courses", href: "/courses.html" },
    { id: 4, iconClass: "fas fa-store", desc: "Yoddha Store", href: "/yoddha-store.html" },
    { id: 5, iconClass: "fas fa-bars", desc: "More", href: "/more.html" },
  ];

  let navbarContent = '<nav class="flex justify-evenly pt-2 pb-1">'; // Reduced padding

  navItems.forEach(item => {
    // Normalize paths for comparison
     const currentPathBase = window.location.pathname.split('/').pop(); // Get 'index.html', 'cart.html', etc.
     const itemPathBase = item.href.split('/').pop();
     const isActive = currentPathBase === itemPathBase || (currentPathBase === '' && itemPathBase === 'index.html'); // Handle root path correctly


    navbarContent += `
      <a href="${item.href}" key="${item.id}" class="flex flex-1 flex-col items-center justify-center text-center w-1/5 py-1">
        <div class="w-5 h-5 mb-1 flex items-center justify-center"> <!-- Fixed size icon container -->
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
 * Renders a generic component's HTML into a target container.
 * @param {string | HTMLElement} containerOrSelector - The CSS selector or the container element itself.
 * @param {string} htmlContent - The HTML string to render.
 */
function renderComponent(containerOrSelector, htmlContent) {
    const container = typeof containerOrSelector === 'string' ? $(containerOrSelector) : containerOrSelector;
    if (container) {
        container.innerHTML = htmlContent;
    } else {
        console.warn(`Container element "${containerOrSelector}" not found.`);
    }
}

/**
 * Renders the Home Page Banner.
 * @param {string} containerSelector - CSS selector for the container element.
 * @param {object} data - Data for the banner (e.g., { bannerTitle: '...' }).
 */
function renderHomeBanner(containerSelector, data) {
    // Example banner - Adapt based on original design
    const htmlContent = `
        <div class="bg-gradient-to-r from-primary via-primary-light to-secondary text-white p-8 rounded-lg shadow-lg text-center">
            <h1 class="text-4xl md:text-6xl font-bold mb-4">${data.bannerTitle || 'Welcome to Veducation'}</h1>
            <p class="text-lg md:text-xl mb-6">Explore ancient wisdom for modern times.</p>
            <a href="/courses.html" class="bg-white text-primary font-bold py-2 px-6 rounded-full hover:bg-gray-100 transition duration-300">
                Explore Courses
            </a>
        </div>
    `;
    renderComponent(containerSelector, htmlContent);
}


/**
 * Renders the About Section on the Home Page.
 * Translates the structure from the AboutPage component.
 * Safely handles Markdown rendering.
 * @param {string | HTMLElement} containerOrSelector - CSS selector or the container element.
 * @param {object} data - About section data { title, greet, desc, img }.
 */
function renderAboutSection(containerOrSelector, data) {
    const container = typeof containerOrSelector === 'string' ? $(containerOrSelector) : containerOrSelector;
    if (!container) {
        console.warn(`About section container "${containerOrSelector}" not found.`);
        return;
    }

    // Ensure data and description exist
    const descriptionText = data?.desc || '';
    let descriptionHtml = '';

    // Safely attempt Markdown rendering
    try {
        if (typeof window.markdownit === 'function') {
            const md = window.markdownit({ html: true, breaks: true }); // Enable HTML and line breaks
            descriptionHtml = md.render(descriptionText);
        } else {
            // Fallback: replace newlines with <br> manually - escape HTML to prevent XSS if source is untrusted
            const escapedText = descriptionText.replace(/</g, "<").replace(/>/g, ">");
            descriptionHtml = `<p>${escapedText.replace(/\n/g, '<br>')}</p>`; // Wrap in paragraph
        }
    } catch (e) {
        console.error("Error rendering markdown:", e);
        // Fallback if markdown rendering itself fails
         const escapedText = descriptionText.replace(/</g, "<").replace(/>/g, ">");
         descriptionHtml = `<p>${escapedText.replace(/\n/g, '<br>')}</p>`;
    }


    const htmlContent = `
        <div class="flex flex-col md:flex-row items-center gap-6 md:gap-10 bg-background p-4 rounded-lg shadow">
            <div class="w-full md:w-2/5 flex-shrink-0">
                <div class="relative aspect-square object-contain rounded-lg overflow-hidden shadow-md">
                    <img src="${data?.img || '/placeholder-image.png'}" alt="${data?.title || 'About'}" class="absolute inset-0 w-full h-full object-cover">
                </div>
                 <div class="mt-[-60px] ml-4 relative z-10 text-left md:mt-[-80px] md:ml-6">
                    <p class="text-xl md:text-2xl text-secondary leading-tight">${data?.greet || ''}</p>
                    <p class="text-xl md:text-2xl text-primary2 font-bold leading-tight max-w-[240px]">${data?.title || ''}</p>
                </div>
            </div>
            <div class="w-full md:w-3/5 mt-[-20px] md:mt-0">
                <div class="prose prose-neutral max-w-none text-base leading-relaxed text-neutral-600">
                   ${descriptionHtml} <!-- Use rendered HTML -->
                </div>
                <div class="mt-5">
                     <a href="/about.html" class="inline-block bg-primary text-white font-bold py-3 px-6 rounded-md hover:bg-primary-dark transition duration-300 text-base w-full text-center md:w-auto">
                         Tell Me More
                     </a>
                </div>
            </div>
        </div>
    `;
    renderComponent(container, htmlContent); // Use the generic renderer
}


/**
 * Renders a list of products (e.g., Books, Store Items).
 * @param {string | HTMLElement} containerOrSelector - CSS selector or the container element.
 * @param {string} title - The title for the product section.
 * @param {string} viewAllLink - The URL for the "View All" link.
 * @param {Array<object>} products - Array of product objects. Expected structure: { id, attributes: { title, author?, price?, posterImageUrl: { data: { attributes: { url } } } } }
 */
function renderProductList(containerOrSelector, title, viewAllLink, products) {
    const container = typeof containerOrSelector === 'string' ? $(containerOrSelector) : containerOrSelector;
    if (!container) {
        console.warn(`Product list container "${containerOrSelector}" not found.`);
        return;
    }

     if (!products || !products.length) {
        container.innerHTML = `<p>No products found in this category.</p>`;
        return;
    }


    let productListHTML = `
        <div class="flex justify-between items-center mb-3">
            <h3 class="text-xl font-bold text-neutral-700 sm:text-xxl md:text-2xl mb-0">${title}</h3>
            ${viewAllLink ? `<a href="${viewAllLink}" class="text-sm text-primary hover:underline">View all</a>` : ''}
        </div>
        <div class="flex overflow-x-auto no-scrollbar gap-2 pb-2 -mx-2 px-2"> <!-- Add padding compensation -->
    `;

    products.forEach(product => {
        const imageUrl = product.attributes?.posterImageUrl?.data?.attributes?.url || '/placeholder-image.png';
        const productTitle = product.attributes?.title || 'Untitled Product';
        const author = product.attributes?.author ? `by ${product.attributes.author}` : '';
        // Example: Use formatPrice utility if available
        const price = typeof formatPrice === 'function' && product.attributes?.price ? formatPrice(product.attributes.price) : (product.attributes?.price ? `$${product.attributes.price.toFixed(2)}` : '');

        productListHTML += `
            <a href="/product.html?id=${product.id}" class="flex-shrink-0 w-[150px] md:w-[180px]">
                <div class="flex flex-col h-full bg-white border border-neutral-200 rounded-md shadow-sm hover:shadow-lg transition-shadow duration-300 overflow-hidden">
                     <div class="relative aspect-[2/3] w-full bg-gray-100"> <!-- Book-like aspect ratio -->
                        <img src="${imageUrl}" alt="${productTitle}" class="absolute inset-0 w-full h-full object-contain p-1"> <!-- Object contain -->
                    </div>
                    <div class="p-3 flex-grow flex flex-col justify-between">
                        <div>
                            <p class="text-neutral-800 text-sm font-semibold leading-tight m-0 pb-1 line-clamp-2">${productTitle}</p>
                            ${author ? `<p class="text-xs font-medium leading-tight text-neutral-500 mb-1">${author}</p>` : ''}
                        </div>
                        <div>
                            <!-- Reviews Placeholder -->
                            <div class="flex items-center text-xs mt-1 text-amber-400">
                               <i class="fas fa-star"></i>
                               <i class="fas fa-star"></i>
                               <i class="fas fa-star"></i>
                               <i class="fas fa-star-half-alt"></i>
                               <i class="far fa-star"></i>
                               <span class="ml-1 text-neutral-500 text-[10px]">(100)</span> <!-- Example count -->
                            </div>
                             ${price ? `<p class="text-primary font-bold text-sm mt-1 mb-0">${price}</p>` : ''}
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
 * Renders a list of Course Cards.
 * @param {string | HTMLElement} containerOrSelector - CSS selector or the container element.
 * @param {string} title - The title for the course section.
 * @param {string} viewAllLink - The URL for the "View All" link.
 * @param {Array<object>} courses - Array of course objects. Expected structure: { id, attributes: { title, description, img } }
 */
function renderCourseList(containerOrSelector, title, viewAllLink, courses) {
    const container = typeof containerOrSelector === 'string' ? $(containerOrSelector) : containerOrSelector;
     if (!container) {
        console.warn(`Course list container "${containerOrSelector}" not found.`);
        return;
    }

    if (!courses || !courses.length) {
        container.innerHTML = `<p>No courses available at the moment.</p>`;
        return;
    }

    let courseListHTML = `
        <div class="flex justify-between items-center mb-3">
            <h3 class="text-xl font-bold text-neutral-700 sm:text-xxl md:text-2xl mb-0">${title}</h3>
            ${viewAllLink ? `<a href="${viewAllLink}" class="text-sm text-primary hover:underline">View all</a>` : ''}
        </div>
        <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4">
    `;

    courses.forEach(course => {
        const imageUrl = course.attributes?.img || '/placeholder-image.png'; // Use attribute directly if not nested
        const courseTitle = course.attributes?.title || 'Untitled Course';
        const courseDesc = course.attributes?.description || '';
        // Example: Add price or other details if available
        const price = typeof formatPrice === 'function' && course.attributes?.price ? formatPrice(course.attributes.price) : (course.attributes?.price ? `$${course.attributes.price.toFixed(2)}` : '');


        // Using a structure similar to the CourseCard component
        courseListHTML += `
            <a href="/course-details.html?id=${course.id}" class="block group"> <!-- Link the whole card -->
                 <div class="bg-white rounded-lg shadow-md overflow-hidden transition-shadow duration-300 group-hover:shadow-xl">
                     <div class="relative aspect-video w-full"> <!-- Aspect ratio for video/image -->
                         <img src="${imageUrl}" alt="${courseTitle}" class="absolute inset-0 w-full h-full object-cover">
                         <div class="absolute inset-0 bg-black bg-opacity-20 group-hover:bg-opacity-10 transition-opacity duration-300"></div>
                          <!-- Optional: Play icon overlay -->
                          <!-- <div class="absolute inset-0 flex items-center justify-center opacity-0 group-hover:opacity-100 transition-opacity duration-300">
                              <i class="fas fa-play-circle text-white text-4xl"></i>
                          </div> -->
                     </div>
                     <div class="p-4">
                         <h4 class="text-lg font-bold text-neutral-800 mb-1 line-clamp-2 group-hover:text-primary transition-colors">${courseTitle}</h4>
                         <p class="text-sm text-neutral-600 line-clamp-3 mb-2">${courseDesc}</p>
                         ${price ? `<p class="text-primary font-bold text-base mt-1 mb-0">${price}</p>` : ''}
                         <!-- Optional: Rating/Progress -->
                     </div>
                 </div>
             </a>
        `;
    });

    courseListHTML += `</div>`;
    container.innerHTML = courseListHTML;
}


/**
 * Renders the Donations Section. Includes featured donation and list.
 * @param {string | HTMLElement} containerOrSelector - CSS selector or the container element.
 * @param {Array<object>} donations - Array of donation campaign objects. Expected: { id, attributes: { title, raised, raisedOutOf, daysRemaining, backers, img } }
 */
function renderDonationsSection(containerOrSelector, donations) {
     const container = typeof containerOrSelector === 'string' ? $(containerOrSelector) : containerOrSelector;
     if (!container) {
        console.warn(`Donations container "${containerOrSelector}" not found.`);
        return;
    }
     if (!donations || !donations.length) {
        container.innerHTML = `<p>No active donation campaigns.</p>`;
        return;
    }

    // --- Featured Donation Card (using the first donation for example) ---
    const featured = donations[0];
    const featuredImg = featured.attributes?.img || '/img1.png'; // Fallback
    const featuredTitle = featured.attributes?.title || 'Help Us Spread Dharma';
    const featuredDesc = `Help us in this mission of spreading this ancient knowledge all over the world. So that we can again make <span class="text-primary2 font-semibold">The Golden Bird</span>.`; // Example description

    let donationsHTML = `
         <div class="flex justify-between items-center mb-3">
            <h3 class="text-xl font-bold text-neutral-700 sm:text-xxl md:text-2xl mb-0">Donations</h3>
            <a href="/donations.html" class="text-sm text-primary hover:underline">View all</a>
        </div>

        <!-- Featured Card -->
        <div class="border border-[#F1DEC7] rounded-md p-3 grid grid-cols-1 md:grid-cols-3 gap-4 mb-6 shadow-sm bg-gradient-to-r from-orange-50 via-amber-50 to-yellow-50">
            <div class="relative aspect-video md:aspect-square rounded overflow-hidden md:col-span-1">
                 <img src="${featuredImg}" alt="${featuredTitle}" class="absolute inset-0 w-full h-full object-cover">
            </div>
            <div class="flex flex-col justify-center md:col-span-2">
                <h4 class="text-xl font-bold text-neutral-800 mb-2 leading-tight">${featuredTitle}</h4>
                <p class="text-base text-neutral-600 mb-4">
                   ${featuredDesc}
                </p>
                <a href="/donations.html#donate-${featured.id}" class="bg-primary text-white rounded-md w-full md:w-auto px-6 py-3 font-bold text-center hover:bg-primary-dark transition duration-300">
                    3 Ways You Can Help Us
                </a>
            </div>
        </div>

        <!-- Donation List (using remaining donations) -->
        <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
    `;

    // --- Donation List Cards ---
    donations.slice(1).forEach(val => { // Start from the second item if the first is featured
        const progressValue = parseInt(String(val.attributes?.raised || '0').replace(/,/g, ''), 10);
        const progressMax = parseInt(String(val.attributes.raisedOutOf || '1').replace(/,/g, ''), 10) || 1; // Avoid division by zero
        const percentage = Math.min(100, Math.round((progressValue / progressMax) * 100));
        const raisedFormatted = typeof formatPrice === 'function' ? formatPrice(progressValue) : `$${val.attributes.raised || '0'}`;
        const goalFormatted = typeof formatPrice === 'function' ? formatPrice(progressMax) : `$${val.attributes.raisedOutOf || '?'}`;

        donationsHTML += `
            <div class="border border-[#F1DEC7] rounded-md p-3 flex flex-col text-[#3A3A3A] shadow-sm bg-white">
                <div class="relative aspect-video mb-2 rounded-sm overflow-hidden">
                     <img src="${val.attributes.img || '/placeholder-image.png'}" class="absolute inset-0 w-full h-full object-cover" alt="donation campaign">
                </div>
                <h5 class="font-bold mb-2 pt-2 leading-tight line-clamp-2">${val.attributes.title}</h5>
                <div class="text-sm text-[#3A3A3A] pb-1 mb-1 flex gap-1 items-center flex-wrap">
                    <span class="font-bold text-base text-primary">${raisedFormatted}</span> raised out of ${goalFormatted}
                </div>

                 <div class="w-full bg-gray-200 rounded-full h-2 mb-2 overflow-hidden">
                     <div class="bg-gradient-to-r from-[#DD2289] to-[#FEBC08] h-2 rounded-full" style="width: ${percentage}%"></div>
                 </div>

                <div class="text-xs flex items-center justify-between pb-4 text-neutral-500">
                     <span class="inline-flex items-center gap-1 bg-[#F7DCBD] px-2 py-1 rounded text-xs font-semibold">
                         <i class="far fa-clock"></i> ${val.attributes.daysRemaining} Days Left
                     </span>
                     <span class="inline-flex items-center gap-1 bg-[#F7DCBD] px-2 py-1 rounded text-xs font-semibold">
                         <i class="fas fa-users"></i> ${val.attributes.backers} backers
                     </span>
                </div>
                <div class="flex gap-2 font-bold mt-auto"> <!-- mt-auto pushes buttons down -->
                    <button class="flex-1 text-sm2 border border-[#F5D9B1] bg-primary3 text-background2 rounded py-2 px-2 hover:opacity-90 transition-opacity">Share</button>
                    <a href="/donations.html#donate-${val.id}" class="flex-1 bg-primary text-white rounded py-2 px-2 text-sm2 text-center hover:bg-primary-dark transition-opacity">Donate Now</a>
                </div>
            </div>
        `;
    });


    donationsHTML += `</div>`; // Close grid container
    container.innerHTML = donationsHTML;
}


/**
 * Renders the list of options for the "More" page.
 * Dynamically shows Login/Logout based on auth state.
 * @param {string | HTMLElement} containerOrSelector - CSS selector or the container element.
 * @param {object | null} user - The current user object or null.
 */
function renderMorePageList(containerOrSelector, user) {
    const container = typeof containerOrSelector === 'string' ? $(containerOrSelector) : containerOrSelector;
     if (!container) {
        console.warn(`More options container "${containerOrSelector}" not found.`);
        return;
    }

    const commonLinks = [
        { href: '/about.html', iconClass: 'fas fa-info-circle', text: 'About Us' },
        { href: '/donations.html', iconClass: 'fas fa-hand-holding-heart', text: 'Donations' },
        { href: '/contact.html', iconClass: 'fas fa-envelope', text: 'Contact Us' }, // Example link
        { href: '/faq.html', iconClass: 'fas fa-question-circle', text: 'FAQ' }, // Example link
        // Add other common links here
    ];

    const loggedInLinks = [
        { href: '/profile.html', iconClass: 'fas fa-user-circle', text: 'My Profile' },
        { href: '/orders.html', iconClass: 'fas fa-box-open', text: 'My Orders' }, // Assuming orders.html
        { href: '/settings.html', iconClass: 'fas fa-cog', text: 'Settings' }, // Assuming settings.html
        // Add other logged-in specific links
    ];

    let linksHTML = '';

    if (user) {
        // Combine common and logged-in links
        [...loggedInLinks, ...commonLinks].forEach(link => {
            linksHTML += `
                <a href="${link.href}" class="more-link-item">
                    <div class="flex items-center">
                         <i class="${link.iconClass} item-icon"></i>
                         <span class="item-text">${link.text}</span>
                    </div>
                    <i class="fas fa-chevron-right"></i>
                </a>
            `;
        });
        // Add Logout button
        linksHTML += `
            <button id="more-logout-button" class="more-link-item w-full text-left text-red-600">
                 <div class="flex items-center">
                    <i class="fas fa-sign-out-alt item-icon"></i>
                    <span class="item-text font-semibold">Logout</span>
                </div>
                 <i class="fas fa-chevron-right"></i>
            </button>
        `;
    } else {
        // Show common links and Login button
         commonLinks.forEach(link => {
            linksHTML += `
                <a href="${link.href}" class="more-link-item">
                     <div class="flex items-center">
                         <i class="${link.iconClass} item-icon"></i>
                         <span class="item-text">${link.text}</span>
                    </div>
                    <i class="fas fa-chevron-right"></i>
                </a>
            `;
        });
         // Add Login button
         linksHTML += `
            <button id="more-login-button" class="more-link-item w-full text-left text-green-600">
                 <div class="flex items-center">
                    <i class="fab fa-github item-icon"></i>
                    <span class="item-text font-semibold">Login with GitHub</span>
                 </div>
                 <i class="fas fa-chevron-right"></i>
            </button>
        `;
    }

    container.innerHTML = linksHTML;

     // Add event listeners for login/logout buttons within the list
     if (user) {
        const logoutBtn = $('#more-logout-button');
        if (logoutBtn) {
            logoutBtn.removeEventListener('click', handleLogout); // Prevent duplicates
            logoutBtn.addEventListener('click', handleLogout); // Defined in auth.js
        }
     } else {
        const loginBtn = $('#more-login-button');
        if (loginBtn) {
             loginBtn.removeEventListener('click', handleLogin); // Prevent duplicates
             loginBtn.addEventListener('click', handleLogin); // Defined in auth.js
         }
     }
}


// --- Helper function (example - defined in app.js or specific cart file) ---
// function getCartItemCount() { /* ... implementation ... */ return 0; }
// function updateCartBadge() { /* ... implementation ... */ }
