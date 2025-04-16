// --- Main Application Logic ---

const VEDUCATION_CART_KEY = 'veducationCart';

// --- State Management (Simple Example using localStorage for Cart) ---

/**
 * Gets the current cart from localStorage.
 * @returns {Array<object>} Array of cart items.
 */
function getCart() {
  const cartJson = localStorage.getItem(VEDUCATION_CART_KEY);
  try {
    return cartJson ? JSON.parse(cartJson) : [];
  } catch (error) {
    console.error("Error parsing cart data:", error);
    localStorage.removeItem(VEDUCATION_CART_KEY); // Clear potentially corrupted data
    return [];
  }
}

/**
 * Saves the cart to localStorage.
 * @param {Array<object>} cart - The cart array to save.
 */
function saveCart(cart) {
  try {
    localStorage.setItem(VEDUCATION_CART_KEY, JSON.stringify(cart));
    updateCartBadge(); // Update header badge whenever cart changes
  } catch (error) {
    console.error("Error saving cart data:", error);
  }
}

/**
 * Adds an item to the cart or increments its quantity.
 * @param {object} product - The product object to add (needs id, title, price, image url).
 * @param {number} [quantity=1] - The quantity to add.
 */
function addToCart(product, quantity = 1) {
  if (!product || !product.id) {
    console.error("Invalid product data for addToCart:", product);
    return;
  }
  let cart = getCart();
  const existingItemIndex = cart.findIndex(item => item.id === product.id);

  if (existingItemIndex > -1) {
    cart[existingItemIndex].quantity = (cart[existingItemIndex].quantity || 1) + quantity;
  } else {
    cart.push({
        id: product.id,
        title: product.title || 'Unknown Item',
        price: product.price || 0,
        image: product.image || '/placeholder-image.png',
        quantity: quantity
     });
  }
  saveCart(cart);
  console.log(`Added ${quantity} of ${product.title} to cart.`, cart);
}

/**
 * Removes an item completely from the cart.
 * @param {number} productId - The ID of the product to remove.
 */
function removeFromCart(productId) {
  let cart = getCart();
  cart = cart.filter(item => item.id !== productId);
  saveCart(cart);
  console.log(`Removed product ${productId} from cart.`, cart);
   if (window.location.pathname.includes('cart.html')) {
       renderCartPageContent();
   }
}

/**
 * Updates the quantity of a specific item in the cart.
 * If quantity is 0 or less, removes the item.
 * @param {number} productId - The ID of the product to update.
 * @param {number} newQuantity - The new quantity.
 */
function updateCartItemQuantity(productId, newQuantity) {
  let cart = getCart();
  const itemIndex = cart.findIndex(item => item.id === productId);

  if (itemIndex > -1) {
    if (newQuantity > 0) {
      cart[itemIndex].quantity = newQuantity;
    } else {
      cart.splice(itemIndex, 1);
    }
    saveCart(cart);
    console.log(`Updated product ${productId} quantity to ${newQuantity}.`, cart);
    if (window.location.pathname.includes('cart.html')) {
        renderCartPageContent();
    }
  }
}

/**
 * Clears the entire cart.
 */
function clearCart() {
    localStorage.removeItem(VEDUCATION_CART_KEY);
    console.log("Cart cleared.");
    updateCartBadge();
    if (window.location.pathname.includes('cart.html')) {
        renderCartPageContent();
    }
}


// --- Routing and Page Initialization ---

/**
 * Loads content based on the current URL path.
 */
async function loadPageContent() {
  const path = window.location.pathname;
  const mainContent = $('#main-content');
  if (!mainContent) return;

  // Clear previous content and show loading state
  mainContent.innerHTML = '<p class="text-center p-10">Loading...</p>';

  if (path === '/' || path === '/index.html') {
    await loadHomePageContent(mainContent);
  } else if (path.startsWith('/product.html')) {
    await loadProductDetailsPageContent(mainContent);
  } else if (path === '/cart.html') {
    await loadCartPageContent(mainContent);
   } else if (path === '/library.html') {
     await loadLibraryPageContent(mainContent);
   } else if (path.startsWith('/readbook.html')) {
     await loadReadBookPageContent(mainContent);
   } else if (path === '/courses.html') {
     await loadCoursesPageContent(mainContent);
   } else if (path.startsWith('/course-details.html')) { // Adjusted path for generic details page
      await loadCourseDetailsPageContent(mainContent);
   } else if (path === '/yoddha-store.html') {
      await loadYoddhaStorePageContent(mainContent);
   } else if (path === '/profile.html') {
      await loadProfilePageContent(mainContent);
   } else if (path === '/login.html') {
       loadLoginPageContent(mainContent);
   } else if (path === '/notifications.html') {
      await loadNotificationsPageContent(mainContent);
   } else if (path === '/orderconfirmed.html') {
        await loadOrderConfirmedPageContent(mainContent);
   } else if (path === '/billingdetails.html') {
        await loadBillingDetailsPageContent(mainContent);
   } else if (path === '/more.html') {
        await loadMorePageContent(mainContent);
   } else if (path === '/about.html') { // Added About page route
        await loadAboutPageContent(mainContent);
   } else if (path === '/donations.html') { // Added Donations page route
        await loadDonationsPageContent(mainContent);
   } else if (path.startsWith('/products.html')) { // Added generic product category page
        await loadProductCategoryPageContent(mainContent);
   }
  else {
    renderComponent(mainContent, `<h2>404 - Page Not Found</h2><p>The requested page ${path} could not be found.</p>`);
  }

  // Re-render navbar AFTER loading content to ensure correct active state
  renderNavbar(window.location.pathname);
}

/**
 * Loads and renders content for the Home Page.
 * @param {HTMLElement} container - The main content container element.
 */
async function loadHomePageContent(container) {
    // Reset container for fresh rendering
    container.innerHTML = `
        <section id="home-banner" class="mb-8"></section>
        <section id="about-section" class="mb-8 px-container md:px-0"></section> <!-- Added padding -->
        <section id="product-list-books" class="mb-8 px-container md:px-0"></section> <!-- Added padding -->
        <section id="product-list-store" class="mb-8 px-container md:px-0"></section> <!-- Added padding -->
        <section id="courses-section" class="mb-8 px-container md:px-0"></section> <!-- Added padding -->
        <section id="donations-section" class="mb-8 px-container md:px-0"></section> <!-- Added padding -->
    `;

    try {
        // Fetch all data concurrently
        const [homeData, aboutData, booksData, storeData, coursesData, donationsData] = await Promise.all([
            fetchHomeData(),
            fetchAboutData(),
            fetchCategoryData('books'), // Make sure 'books' matches key in api.js
            fetchCategoryData('store'), // Make sure 'store' matches key in api.js
            fetchCoursesData(),
            fetchDonationsData()
        ]);

        // Render each section using the fetched data
        renderHomeBanner('#home-banner', homeData);
        renderAboutSection('#about-section', aboutData); // Pass data to the new function
        renderProductList('#product-list-books', booksData.name || 'Featured Books', '/products.html?category=books', booksData.products || []);
        renderProductList('#product-list-store', storeData.name || 'Yoddha Store', '/products.html?category=store', storeData.products || []);
        renderCourseList('#courses-section', 'Featured Courses', '/courses.html', coursesData || []);
        renderDonationsSection('#donations-section', donationsData || []); // Pass data

    } catch (error) {
        console.error("Error loading home page content:", error);
        renderComponent(container, `<p class="text-red-500 text-center p-10">Error loading content. Please try again later.</p>`);
    }
}

/**
 * Loads and renders content for the Product Details Page.
 * @param {HTMLElement} container - The main content container element.
 */
async function loadProductDetailsPageContent(container) {
    const productId = getQueryParam('id'); // Use utility function
    if (!productId) {
        renderComponent(container, `<p>Invalid product ID.</p>`);
        return;
    }

    renderComponent(container, `<p class="text-center p-10">Loading product details...</p>`); // Loading state

    try {
        const product = await fetchProductDetails(productId); // from api.js (simulated)
        if (!product) {
             renderComponent(container, `<p>Product not found.</p>`);
             return;
        }
        const imageUrl = product.attributes?.posterImageUrl?.data?.attributes?.url || '/placeholder-image.png';
        const priceFormatted = typeof formatPrice === 'function' ? formatPrice(product.attributes.price) : `$${product.attributes.price?.toFixed(2) || 'N/A'}`;

        // Render Product Detail Structure
        renderComponent(container, `
            <div class="flex flex-col md:flex-row gap-6 md:gap-10 px-container py-4">
                <div class="md:w-1/3 flex justify-center">
                     <img src="${imageUrl}" alt="${product.attributes.title}" class="max-w-[250px] md:max-w-full w-auto h-auto max-h-[400px] object-contain rounded shadow-lg border">
                </div>
                <div class="md:w-2/3">
                    <h1 class="text-2xl md:text-3xl font-bold mb-1">${product.attributes.title}</h1>
                    ${product.attributes.author ? `<p class="text-md text-gray-600 mb-3">by ${product.attributes.author}</p>` : ''}
                    <p class="text-2xl font-bold text-primary mb-3">${priceFormatted}</p>
                     <!-- Reviews Placeholder -->
                     <div class="flex items-center text-amber-400 mb-4">
                        <i class="fas fa-star"></i><i class="fas fa-star"></i><i class="fas fa-star"></i><i class="fas fa-star-half-alt"></i><i class="far fa-star"></i>
                        <a href="#reviews" class="ml-2 text-sm text-neutral-500 hover:underline">(123 Reviews)</a> <!-- Example -->
                     </div>
                     <!-- Description -->
                     <h3 class="font-bold text-sm2 pt-4 pb-2 border-t">Description</h3>
                     <div class="prose prose-sm max-w-none mb-6 text-neutral-700">
                        ${product.attributes.description || 'No description available.'}
                     </div>
                     <!-- Add/Remove Buttons Section -->
                     <div class="flex items-center gap-4 mb-6">
                         <label for="quantity" class="font-semibold">Quantity:</label>
                         <div class="flex items-center border rounded">
                            <button class="quantity-change bg-gray-200 px-3 py-1 rounded-l" data-change="-1">-</button>
                            <input type="number" id="quantity" name="quantity" value="1" min="1" class="w-12 text-center border-y focus:outline-none" readonly>
                            <button class="quantity-change bg-gray-200 px-3 py-1 rounded-r" data-change="1">+</button>
                         </div>
                     </div>
                     <button id="add-to-cart-btn" class="w-full md:w-auto bg-primary text-white font-bold py-3 px-8 rounded hover:bg-primary-dark transition duration-300">
                        Add to Cart
                    </button>
                </div>
            </div>
            <!-- Reviews Section Placeholder -->
            <section id="reviews" class="mt-8 border-t pt-6 px-container">
                <h2 class="text-xl font-bold mb-4">Customer Reviews</h2>
                <p>Reviews section coming soon...</p>
            </section>
            <!-- Related Products Placeholder -->
            <section id="related-products" class="mt-8 border-t pt-6 px-container">
                 <h2 class="text-xl font-bold mb-4">Related Products</h2>
                <p>Loading related products...</p>
            </section>
        `);

        // Add event listener logic
        const quantityInput = container.querySelector('#quantity');
        container.querySelectorAll('.quantity-change').forEach(button => {
            button.addEventListener('click', () => {
                const change = parseInt(button.dataset.change);
                const currentValue = parseInt(quantityInput.value);
                const newValue = Math.max(1, currentValue + change); // Ensure quantity is at least 1
                quantityInput.value = newValue;
            });
        });

        container.querySelector('#add-to-cart-btn').addEventListener('click', () => {
            const quantity = parseInt(quantityInput.value);
            addToCart({
                id: product.id,
                title: product.attributes.title,
                price: product.attributes.price,
                image: imageUrl
            }, quantity);
            alert(`${quantity} x ${product.attributes.title} added to cart!`);
        });

        // Optionally load related products
        // const related = await fetchRelatedProducts(productId); // from api.js
        // renderProductList('#related-products', '', '', related);
        $('#related-products').innerHTML = '<p>Related products loading soon.</p>';


    } catch (error) {
        console.error("Error loading product details:", error);
        renderComponent(container, `<p class="text-red-500 text-center p-10">Error loading product details.</p>`);
    }
}


/**
 * Loads and renders content for the Cart Page.
 * @param {HTMLElement} container - The main content container element.
 */
async function loadCartPageContent(container) {
     // Initial structure for cart page
     container.innerHTML = `
        <div class="px-container py-4">
            <h1 class="text-2xl font-bold mb-6">Shopping Cart</h1>
            <div id="cart-items-container" class="mb-6">
                <!-- Cart items will be rendered here -->
            </div>
            <div id="cart-summary-container">
                <!-- Cart summary and checkout button will be rendered here -->
            </div>
        </div>`;
     renderCartPageContent(); // Call the rendering function to populate items/summary
}

// --- Add loader functions for other pages ---
async function loadLibraryPageContent(container) { renderComponent(container, `<h2>Library</h2><p>Content coming soon...</p>`); }
async function loadReadBookPageContent(container) { renderComponent(container, `<h2>Read Book</h2><p>Content coming soon...</p>`); }
async function loadCoursesPageContent(container) { renderComponent(container, `<h2>Courses</h2><p>Content coming soon...</p>`); }
async function loadCourseDetailsPageContent(container) { renderComponent(container, `<h2>Course Details</h2><p>Content coming soon...</p>`); }
async function loadYoddhaStorePageContent(container) { renderComponent(container, `<h2>Yoddha Store</h2><p>Content coming soon...</p>`); }
async function loadProfilePageContent(container) { renderComponent(container, `<h2>Profile</h2><p>Content coming soon...</p>`); }
function loadLoginPageContent(container) { renderComponent(container, `<h2>Login</h2><p>Please click the login button in the header.</p>`); }
async function loadNotificationsPageContent(container) { renderComponent(container, `<h2>Notifications</h2><p>Content coming soon...</p>`); }
async function loadOrderConfirmedPageContent(container) { renderComponent(container, `<h2>Order Confirmed</h2><p>Content coming soon...</p>`); }
async function loadBillingDetailsPageContent(container) { renderComponent(container, `<h2>Billing Details</h2><p>Content coming soon...</p>`); }
async function loadMorePageContent(container) { renderComponent(container, `<h2>More</h2><p>Content coming soon...</p>`); }
async function loadAboutPageContent(container) {
    renderComponent(container, `<p class="text-center p-10">Loading About Page...</p>`);
    try {
        const aboutData = await fetchAboutData(); // Fetch data
        renderAboutSection(container, aboutData); // Reuse the section renderer
    } catch(error) {
        renderComponent(container, `<p class="text-red-500 text-center p-10">Error loading about page.</p>`);
    }
}
async function loadDonationsPageContent(container) {
    renderComponent(container, `<p class="text-center p-10">Loading Donations Page...</p>`);
     try {
        const donationsData = await fetchDonationsData();
        // Need a dedicated function in ui.js to render the full donations page layout
        renderFullDonationsPage(container, donationsData); // Assuming this function exists in ui.js
    } catch(error) {
        renderComponent(container, `<p class="text-red-500 text-center p-10">Error loading donations page.</p>`);
    }
}
async function loadProductCategoryPageContent(container) {
     renderComponent(container, `<p class="text-center p-10">Loading Products...</p>`);
     const categoryId = getQueryParam('category') || 'all'; // Get category from URL or default
     try {
        const categoryData = await fetchCategoryData(categoryId); // Fetch data for the specific category
         // Need a dedicated function in ui.js to render a grid or list of products
         renderProductGridPage(container, categoryData); // Assuming this function exists in ui.js
     } catch(error) {
         renderComponent(container, `<p class="text-red-500 text-center p-10">Error loading products.</p>`);
     }
}

// Placeholder for full donations page renderer (add to ui.js)
function renderFullDonationsPage(container, donations) {
     renderComponent(container, `<h2>Donations Page</h2><p>Rendering full donations list soon...</p>`);
     // Add logic similar to renderDonationsSection but for all items in a grid/list format
}
// Placeholder for product grid page renderer (add to ui.js)
function renderProductGridPage(container, categoryData) {
     renderComponent(container, `<h2>${categoryData.name || 'Products'}</h2><p>Rendering product grid soon...</p>`);
     // Add logic to display categoryData.products in a grid
}


/**
 * Initializes the page: renders header/navbar, loads content based on URL.
 */
function initializePage() {
  console.log("Initializing page...");
  const user = getCurrentUser();
  renderHeader(user);
  // Navbar rendering moved to the end of loadPageContent to ensure correct active state
  loadPageContent();
}

// --- Global Initialization ---
document.addEventListener('DOMContentLoaded', initializePage);

// --- Handle Browser Navigation (Back/Forward buttons) ---
window.addEventListener('popstate', loadPageContent);

// ...(previous code in app.js)...

// --- Routing and Page Initialization ---

/**
 * Loads content based on the current URL path.
 */
async function loadPageContent() {
  const path = window.location.pathname;
  const mainContent = $('#main-content');
  if (!mainContent) return;

  // Clear previous content and show loading state
  mainContent.innerHTML = '<p class="text-center p-10">Loading...</p>';

  if (path === '/' || path === '/index.html') {
    await loadHomePageContent(mainContent);
  } else if (path.startsWith('/product.html')) {
    await loadProductDetailsPageContent(mainContent);
  } else if (path === '/cart.html') {
    await loadCartPageContent(mainContent);
   } else if (path === '/library.html') {
     await loadLibraryPageContent(mainContent);
   } else if (path.startsWith('/readbook.html')) {
     await loadReadBookPageContent(mainContent);
   } else if (path === '/courses.html') {
     await loadCoursesPageContent(mainContent);
   } else if (path.startsWith('/course-details.html')) {
      await loadCourseDetailsPageContent(mainContent);
   } else if (path === '/yoddha-store.html') {
      await loadYoddhaStorePageContent(mainContent);
   } else if (path === '/profile.html') {
      await loadProfilePageContent(mainContent); // Placeholder exists
   } else if (path === '/login.html') {
       loadLoginPageContent(mainContent);
   } else if (path === '/notifications.html') {
      await loadNotificationsPageContent(mainContent);
   } else if (path === '/orderconfirmed.html') {
        await loadOrderConfirmedPageContent(mainContent);
   } else if (path === '/billingdetails.html') {
        await loadBillingDetailsPageContent(mainContent);
   } else if (path === '/more.html') {
        await loadMorePageContent(mainContent); // Call the new loader
   } else if (path === '/about.html') {
        await loadAboutPageContent(mainContent);
   } else if (path === '/donations.html') {
        await loadDonationsPageContent(mainContent);
   } else if (path.startsWith('/products.html')) {
        await loadProductCategoryPageContent(mainContent);
   } else if (path === '/orders.html') { // Added route for orders
        await loadOrderHistoryPageContent(mainContent); // Assuming a function to load orders
   } else if (path === '/settings.html') { // Added route for settings
        await loadSettingsPageContent(mainContent); // Assuming a function to load settings
   } else if (path === '/contact.html') { // Added route for contact
        await loadContactPageContent(mainContent); // Assuming a function to load contact
   } else if (path === '/faq.html') { // Added route for faq
        await loadFaqPageContent(mainContent); // Assuming a function to load faq
   }
  else {
    renderComponent(mainContent, `<h2>404 - Page Not Found</h2><p>The requested page ${path} could not be found.</p>`);
  }

  // Re-render navbar AFTER loading content to ensure correct active state
  renderNavbar(window.location.pathname);
}

// ...(loadHomePageContent, loadProductDetailsPageContent, etc.)...

/**
 * Loads and renders content for the More Page.
 * @param {HTMLElement} container - The main content container element.
 */
async function loadMorePageContent(container) {
    const user = getCurrentUser(); // Check login status from auth.js
    // Render the list using the function from ui.js
    renderMorePageList('#more-options-list', user); // Pass the user status
}

// --- Add loader functions for other pages linked from "More" ---
async function loadOrderHistoryPageContent(container) { renderComponent(container, `<h2>Order History</h2><p>Content coming soon...</p>`); }
async function loadSettingsPageContent(container) { renderComponent(container, `<h2>Settings</h2><p>Content coming soon...</p>`); }
async function loadContactPageContent(container) { renderComponent(container, `<h2>Contact Us</h2><p>Content coming soon...</p>`); }
async function loadFaqPageContent(container) { renderComponent(container, `<h2>FAQ</h2><p>Content coming soon...</p>`); }
async function loadProfilePageContent(container) {
    const user = getCurrentUser();
    if (user) {
        // Fetch actual profile/billing data if needed from api.js
        // const profileData = await fetchUserProfile(user.id);
        // const billingData = await fetchBillingDetails(user.id);

        renderComponent('#profile-content-area', `
            <div class="flex flex-col items-center md:items-start md:flex-row gap-6 p-4 bg-white rounded shadow border">
                <img src="${user.avatar_url || '/icons/profile.svg'}" alt="${user.login}" class="w-24 h-24 rounded-full border-2 border-primary">
                <div>
                    <h2 class="text-xl font-bold">${user.name || user.login}</h2>
                    <p class="text-neutral-600">${user.email || 'No email provided'}</p>
                    <!-- Display other profile info here -->
                    <button class="mt-4 text-sm text-blue-600 hover:underline">Edit Profile (Not implemented)</button>
                </div>
            </div>
        `);
        // Load order history (placeholder)
        $('#orders-list').innerHTML = `<p>Order history loading soon...</p>`;
    } else {
         renderComponent('#profile-content-area', `
            <div class="text-center p-6 bg-white rounded shadow border">
                <p class="mb-4">You need to be logged in to view your profile.</p>
                 <button id="profile-login-button" class="bg-primary text-white text-sm font-bold py-2 px-4 rounded hover:bg-primary-dark">
                     <i class="fab fa-github mr-2"></i> Login with GitHub
                 </button>
             </div>
         `);
         // Hide order history if not logged in
          $('#order-history-section').style.display = 'none';

         const loginBtn = $('#profile-login-button');
         if (loginBtn) {
            loginBtn.removeEventListener('click', handleLogin); // Prevent duplicates
            loginBtn.addEventListener('click', handleLogin);
         }
    }
 }

// ...(rest of app.js, including initializePage, event listeners)...
