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
    // Increment quantity
    cart[existingItemIndex].quantity = (cart[existingItemIndex].quantity || 1) + quantity;
  } else {
    // Add new item
    cart.push({
        id: product.id,
        title: product.title || 'Unknown Item',
        price: product.price || 0, // Ensure price is stored correctly
        image: product.image || '/placeholder-image.png', // Store image URL
        quantity: quantity
     });
  }
  saveCart(cart);
  console.log(`Added ${quantity} of ${product.title} to cart.`, cart);
  // Potentially show a confirmation message to the user
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
  // If on cart page, re-render cart UI
   if (window.location.pathname.includes('cart.html')) {
       renderCartPageContent(); // Assumes this function exists for the cart page
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
      // Remove item if quantity is 0 or less
      cart.splice(itemIndex, 1);
    }
    saveCart(cart);
    console.log(`Updated product ${productId} quantity to ${newQuantity}.`, cart);
    // If on cart page, re-render cart UI
    if (window.location.pathname.includes('cart.html')) {
        renderCartPageContent(); // Assumes this function exists for the cart page
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
    // If on cart page, re-render cart UI
    if (window.location.pathname.includes('cart.html')) {
        renderCartPageContent(); // Assumes this function exists for the cart page
    }
}


// --- Routing and Page Initialization ---

/**
 * Loads content based on the current URL path.
 * This is a simple client-side router simulation.
 */
async function loadPageContent() {
  const path = window.location.pathname;
  const mainContent = $('#main-content');
  if (!mainContent) return;

  // Simple routing logic
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
   } else if (path.startsWith('/course/')) { // Example for dynamic course route
      await loadCourseDetailsPageContent(mainContent);
   } else if (path === '/yoddha-store.html') {
      await loadYoddhaStorePageContent(mainContent);
   } else if (path === '/profile.html') {
      await loadProfilePageContent(mainContent);
   } else if (path === '/login.html') { // Could be a dedicated login page
       loadLoginPageContent(mainContent);
   } else if (path === '/notifications.html') {
      await loadNotificationsPageContent(mainContent);
   } else if (path === '/orderconfirmed.html') {
        await loadOrderConfirmedPageContent(mainContent);
   } else if (path === '/billingdetails.html') {
        await loadBillingDetailsPageContent(mainContent);
   } else if (path === '/more.html') {
        await loadMorePageContent(mainContent); // Example
   }
  // Add more routes as needed (e.g., /courses, /library, /profile)
  else {
    mainContent.innerHTML = `<h2>404 - Page Not Found</h2><p>The requested page ${path} could not be found.</p>`;
  }
}

/**
 * Loads and renders content for the Home Page.
 * @param {HTMLElement} container - The main content container element.
 */
async function loadHomePageContent(container) {
    container.innerHTML = `
        <section id="home-banner" class="mb-8"><p>Loading Banner...</p></section>
        <section id="about-section" class="mb-8"><p>Loading About Section...</p></section>
        <section id="product-list-books" class="mb-8"><p>Loading Books...</p></section>
        <section id="product-list-store" class="mb-8"><p>Loading Store Items...</p></section>
        <section id="courses-section" class="mb-8"><p>Loading Courses...</p></section>
        <section id="donations-section" class="mb-8"><p>Loading Donations...</p></section>
    `;

    try {
        // Simulate fetching data (replace with actual API calls if needed)
        const [homeData, aboutData, booksData, storeData, coursesData, donationsData] = await Promise.all([
            fetchHomeData(), // From api.js (simulated)
            fetchAboutData(), // From api.js (simulated)
            fetchCategoryData('books'), // From api.js (simulated) - Assuming category ID/slug 'books'
            fetchCategoryData('store'), // From api.js (simulated) - Assuming category ID/slug 'store'
            fetchCoursesData(), // From api.js (simulated)
            fetchDonationsData() // From api.js (simulated)
        ]);

        // Render Banner (assuming a specific function or using renderComponent)
        renderComponent('#home-banner', `<div><h2>${homeData.bannerTitle || 'Welcome'}</h2></div>`); // Replace with actual banner rendering

        // Render About Section
        renderComponent('#about-section', `
            <div class="bg-gray-100 p-6 rounded shadow">
                <h2 class="text-2xl font-bold mb-4">${aboutData.title || 'About Us'}</h2>
                <div class="flex flex-col md:flex-row gap-4 items-center">
                    <img src="${aboutData.img || '/placeholder-image.png'}" alt="About" class="w-full md:w-1/3 rounded">
                    <p>${aboutData.desc || 'Loading description...'}</p>
                </div>
                <a href="/about.html" class="inline-block mt-4 bg-primary text-white py-2 px-4 rounded hover:bg-primary-dark">Learn More</a>
            </div>
        `);

        // Render Product Lists
        renderProductList('#product-list-books', booksData.name || 'Books', '/products.html?category=books', booksData.products || []);
        renderProductList('#product-list-store', storeData.name || 'Yoddha Store', '/products.html?category=store', storeData.products || []);

        // Render Courses Section (needs a dedicated render function like renderProductList)
        renderCourseList('#courses-section', 'Featured Courses', '/courses.html', coursesData || []);

        // Render Donations Section (needs a dedicated render function)
        renderDonationsSection('#donations-section', donationsData || []);

    } catch (error) {
        console.error("Error loading home page content:", error);
        container.innerHTML = `<p class="text-red-500">Error loading content. Please try again later.</p>`;
    }
}


/**
 * Loads and renders content for the Product Details Page.
 * @param {HTMLElement} container - The main content container element.
 */
async function loadProductDetailsPageContent(container) {
    const params = new URLSearchParams(window.location.search);
    const productId = params.get('id');
    if (!productId) {
        container.innerHTML = `<p>Invalid product ID.</p>`;
        return;
    }

    container.innerHTML = `<p>Loading product details...</p>`; // Loading state

    try {
        const product = await fetchProductDetails(productId); // from api.js (simulated)
        if (!product) {
             container.innerHTML = `<p>Product not found.</p>`;
             return;
        }
        const imageUrl = product.attributes?.posterImageUrl?.data?.attributes?.url || '/placeholder-image.png';

        // Simplified Product Detail Structure - adapt from original component
        renderComponent(container.id, `
            <div class="flex flex-col md:flex-row gap-8">
                <div class="md:w-1/3">
                     <img src="${imageUrl}" alt="${product.attributes.title}" class="w-full rounded shadow-lg">
                </div>
                <div class="md:w-2/3">
                    <h1 class="text-3xl font-bold mb-2">${product.attributes.title}</h1>
                    ${product.attributes.author ? `<p class="text-lg text-gray-600 mb-4">by ${product.attributes.author}</p>` : ''}
                    <p class="text-2xl font-bold text-primary mb-4">$${product.attributes.price || 'N/A'}</p>
                     <!-- Reviews Placeholder -->
                     <div class="flex items-center text-amber-500 mb-4">
                        <i class="fas fa-star"></i><i class="fas fa-star"></i><i class="fas fa-star"></i><i class="fas fa-star-half-alt"></i><i class="far fa-star"></i>
                        <span class="ml-2 text-neutral-500">(Reviews)</span>
                     </div>
                    <div class="prose max-w-none mb-6">
                        ${product.attributes.description || 'No description available.'}
                    </div>
                     <button id="add-to-cart-btn" class="bg-primary text-white font-bold py-3 px-6 rounded hover:bg-primary-dark transition duration-300">
                        Add to Cart
                    </button>
                </div>
            </div>
            <div class="mt-8 border-t pt-6">
                <h3 class="text-xl font-bold mb-4">Related Products</h3>
                <div id="related-products">Loading related products...</div>
            </div>
        `);

        // Add event listener for the add to cart button
        const addToCartBtn = $('#add-to-cart-btn');
        if(addToCartBtn) {
            addToCartBtn.addEventListener('click', () => {
                addToCart({
                    id: product.id,
                    title: product.attributes.title,
                    price: product.attributes.price, // Make sure price format matches cart needs
                    image: imageUrl
                });
                alert(`${product.attributes.title} added to cart!`); // Simple confirmation
            });
        }

        // Optionally load related products
        // const related = await fetchRelatedProducts(productId);
        // renderProductList('#related-products', '', '', related);

    } catch (error) {
        console.error("Error loading product details:", error);
        container.innerHTML = `<p class="text-red-500">Error loading product details.</p>`;
    }
}

/**
 * Loads and renders content for the Cart Page.
 * @param {HTMLElement} container - The main content container element.
 */
async function loadCartPageContent(container) {
     container.innerHTML = `<h1 class="text-2xl font-bold mb-4">Shopping Cart</h1><div id="cart-items-container"></div><div id="cart-summary-container"></div>`;
     renderCartPageContent(); // Call the rendering function
}

/**
 * Renders the actual cart items and summary. Separated to allow re-rendering.
 */
function renderCartPageContent() {
    const itemsContainer = $('#cart-items-container');
    const summaryContainer = $('#cart-summary-container');
    const cart = getCart();

    if (!itemsContainer || !summaryContainer) return; // Ensure containers exist

    if (cart.length === 0) {
        itemsContainer.innerHTML = '<p>Your cart is empty.</p>';
        summaryContainer.innerHTML = ''; // Clear summary if cart is empty
        return;
    }

    // Render Cart Items
    let itemsHTML = '';
    cart.forEach(item => {
        itemsHTML += `
            <div class="flex items-center gap-4 border-b py-4" data-product-id="${item.id}">
                <img src="${item.image || '/placeholder-image.png'}" alt="${item.title}" class="w-20 h-20 object-contain border rounded">
                <div class="flex-grow">
                    <h3 class="text-lg font-semibold">${item.title}</h3>
                    <p class="text-primary font-bold">$${item.price?.toFixed(2) || '0.00'}</p>
                </div>
                <div class="flex items-center gap-2">
                    <button class="cart-quantity-decrease bg-gray-200 px-2 py-1 rounded hover:bg-gray-300">-</button>
                    <span class="font-semibold">${item.quantity}</span>
                    <button class="cart-quantity-increase bg-gray-200 px-2 py-1 rounded hover:bg-gray-300">+</button>
                </div>
                <button class="cart-remove-item text-red-500 hover:text-red-700">
                    <i class="fas fa-trash"></i>
                </button>
            </div>
        `;
    });
    itemsContainer.innerHTML = itemsHTML;

     // Render Cart Summary (Simplified)
     const subtotal = cart.reduce((sum, item) => sum + (item.price * item.quantity), 0);
     const tax = subtotal * 0.10; // Example 10% tax
     const total = subtotal + tax;

     summaryContainer.innerHTML = `
         <div class="mt-6 p-4 border rounded bg-gray-50">
             <h2 class="text-xl font-bold mb-4">Order Summary</h2>
             <div class="flex justify-between mb-2">
                 <span>Subtotal:</span>
                 <span>$${subtotal.toFixed(2)}</span>
             </div>
             <div class="flex justify-between mb-2">
                 <span>Tax (10%):</span>
                 <span>$${tax.toFixed(2)}</span>
             </div>
             <hr class="my-2">
             <div class="flex justify-between font-bold text-lg mb-4">
                 <span>Total:</span>
                 <span>$${total.toFixed(2)}</span>
             </div>
             <a href="/billingdetails.html" class="block w-full text-center bg-primary text-white font-bold py-3 px-6 rounded hover:bg-primary-dark transition duration-300">
                 Proceed to Checkout
             </a>
             <button id="clear-cart-btn" class="block w-full text-center mt-2 text-sm text-red-600 hover:underline">Clear Cart</button>
         </div>
     `;

    // Add event listeners for cart interactions
    $$('.cart-quantity-decrease').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const itemDiv = e.target.closest('[data-product-id]');
            const productId = parseInt(itemDiv.dataset.productId);
            const currentItem = cart.find(i => i.id === productId);
            if (currentItem) {
                updateCartItemQuantity(productId, currentItem.quantity - 1);
            }
        });
    });

    $$('.cart-quantity-increase').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const itemDiv = e.target.closest('[data-product-id]');
            const productId = parseInt(itemDiv.dataset.productId);
             const currentItem = cart.find(i => i.id === productId);
             if (currentItem) {
                 updateCartItemQuantity(productId, currentItem.quantity + 1);
             }
        });
    });

    $$('.cart-remove-item').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const itemDiv = e.target.closest('[data-product-id]');
            const productId = parseInt(itemDiv.dataset.productId);
            if (confirm('Are you sure you want to remove this item?')) {
                removeFromCart(productId);
            }
        });
    });

    const clearCartBtn = $('#clear-cart-btn');
    if (clearCartBtn) {
        clearCartBtn.addEventListener('click', () => {
            if (confirm('Are you sure you want to clear your entire cart?')) {
                clearCart();
            }
        });
    }
}

// --- Add loader functions for other pages (Library, Courses, Profile, etc.) ---
async function loadLibraryPageContent(container) { container.innerHTML = `<h2>Library Page</h2><p>Content coming soon...</p>`; }
async function loadReadBookPageContent(container) { container.innerHTML = `<h2>Read Book Page</h2><p>Content coming soon...</p>`; }
async function loadCoursesPageContent(container) { container.innerHTML = `<h2>Courses Page</h2><p>Content coming soon...</p>`; }
async function loadCourseDetailsPageContent(container) { container.innerHTML = `<h2>Course Details Page</h2><p>Content coming soon...</p>`; }
async function loadYoddhaStorePageContent(container) { container.innerHTML = `<h2>Yoddha Store Page</h2><p>Content coming soon...</p>`; }
async function loadProfilePageContent(container) { container.innerHTML = `<h2>Profile Page</h2><p>Content coming soon...</p>`; }
function loadLoginPageContent(container) { container.innerHTML = `<h2>Login Page</h2><p>Please click the login button in the header.</p>`; }
async function loadNotificationsPageContent(container) { container.innerHTML = `<h2>Notifications Page</h2><p>Content coming soon...</p>`; }
async function loadOrderConfirmedPageContent(container) { container.innerHTML = `<h2>Order Confirmed</h2><p>Content coming soon...</p>`; }
async function loadBillingDetailsPageContent(container) { container.innerHTML = `<h2>Billing Details</h2><p>Content coming soon...</p>`; }
async function loadMorePageContent(container) { container.innerHTML = `<h2>More Options</h2><p>Content coming soon...</p>`; }


// --- Placeholder functions for data fetching (to be defined in api.js) ---
// These will eventually use fetch() or return static data
async function fetchHomeData() { return { bannerTitle: "Welcome to Veducation" }; }
async function fetchAboutData() { return { title: "About Veducation", desc: "Learn about vedic science...", img: "/AboutPageImg.png"}; }
async function fetchCategoryData(categoryId) { /* Placeholder */ return { name: `Category: ${categoryId}`, products: [] }; }
async function fetchCoursesData() { /* Placeholder */ return []; }
async function fetchDonationsData() { /* Placeholder */ return []; }
async function fetchProductDetails(productId) { /* Placeholder */ return null; }
// Add other necessary fetch functions

// --- Placeholder rendering functions (to be defined or refined in ui.js) ---
function renderCourseList(containerSelector, title, viewAllLink, courses) { /* Placeholder */ $(containerSelector).innerHTML = `<p>${title} content coming soon.</p>`; }
function renderDonationsSection(containerSelector, donations) { /* Placeholder */ $(containerSelector).innerHTML = `<p>Donations content coming soon.</p>`; }

/**
 * Initializes the page: renders header/navbar, loads content based on URL.
 * Also used by auth.js to refresh UI after login/logout.
 */
function initializePage() {
  console.log("Initializing page...");
  const user = getCurrentUser(); // Check auth status
  renderHeader(user);           // Render header based on status
  renderNavbar(window.location.pathname); // Render navbar, highlighting active
  loadPageContent();            // Load the main content for the current route
}

// --- Global Initialization ---
// Run initializePage when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', initializePage);
