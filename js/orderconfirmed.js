// --- Order Confirmation Page Logic ---

/**
 * Fetches (simulated) and renders the order confirmation details.
 */
async function loadOrderConfirmation() {
    console.log("Loading order confirmation details...");

    const successMsgContainer = $('#order-success-message');
    const detailsContainer = $('#order-details-content');
    const itemsContainer = $('#purchased-items-content');
    const shippingContainer = $('#shipping-address-content');

    // Clear any previous content / Show loading states
    if(successMsgContainer) successMsgContainer.innerHTML = '<p>Loading confirmation...</p>';
    if(detailsContainer) detailsContainer.innerHTML = '<p>Loading details...</p>';
    if(itemsContainer) itemsContainer.innerHTML = '<p>Loading items...</p>';
    if(shippingContainer) shippingContainer.innerHTML = '<p>Loading address...</p>';

    // Get order details from URL parameters
    const orderId = getQueryParam('orderId'); // From utils.js
    const status = getQueryParam('status');

    if (status !== 'success' || !orderId) {
        // Handle cases where payment might have failed or ID is missing
        if (successMsgContainer) {
             successMsgContainer.innerHTML = `
                <p class="text-red-600">Error processing order.</p>
                <span class="title block text-red-700">There was an issue with your order.</span>
                <p class="arrival-info mt-2">Please contact support or try again.</p>
            `;
        }
        // Clear other sections
        if(detailsContainer) detailsContainer.innerHTML = '';
        if(itemsContainer) itemsContainer.innerHTML = '';
        if(shippingContainer) shippingContainer.innerHTML = '';
        return;
    }

    try {
        // Simulate fetching order details using the ID
        // This function needs to be added to api.js and return { orderInfo, items, shippingInfo }
        const orderData = await fetchOrderDetails(orderId); // Assume this exists in api.js

        if (!orderData) {
            throw new Error("Order data not found.");
        }

        const { orderInfo, items, shippingInfo } = orderData;

        // --- Render Success Message ---
        if (successMsgContainer) {
            // Example arrival date (replace with real logic if possible)
            const arrivalDate = new Date();
            arrivalDate.setDate(arrivalDate.getDate() + 5); // Add 5 days
            const formattedArrivalDate = arrivalDate.toLocaleDateString('en-US', {
                day: 'numeric', month: 'short', year: 'numeric'
            });

             successMsgContainer.innerHTML = `
                <p>Thank you! 🎉</p>
                <span class="title">Your order has been received</span>
                <p class="arrival-info">
                    Estimated arrival by:
                    <span class="arrival-date">${formattedArrivalDate}</span>
                </p>
            `;
        }

        // --- Render Order Details ---
        if (detailsContainer) {
            const orderDate = new Date(orderInfo.date || Date.now()).toLocaleDateString('en-US', {
                 day: 'numeric', month: 'long', year: 'numeric'
            });
            const orderTotalFormatted = typeof formatPrice === 'function' ? formatPrice(orderInfo.total) : `$${orderInfo.total?.toFixed(2) || 'N/A'}`;

            detailsContainer.innerHTML = `
                <div class="order-details-item">
                    <span class="order-details-label">Order Number:</span>
                    <span class="order-details-value">#${orderInfo.id || orderId}</span>
                </div>
                <div class="order-details-item">
                    <span class="order-details-label">Date:</span>
                    <span class="order-details-value">${orderDate}</span>
                </div>
                 <div class="order-details-item">
                    <span class="order-details-label">Total:</span>
                    <span class="order-details-value">${orderTotalFormatted}</span>
                </div>
                 <div class="order-details-item">
                    <span class="order-details-label">Payment Method:</span>
                    <span class="order-details-value">${orderInfo.paymentMethod || 'GitHub Sim / Card'}</span> <!-- Placeholder -->
                </div>
            `;
        }

        // --- Render Purchased Items ---
        if (itemsContainer) {
            if (items && items.length > 0) {
                 let itemsHtml = '';
                 items.forEach(item => {
                    const itemPriceFormatted = typeof formatPrice === 'function' ? formatPrice(item.price) : `$${item.price?.toFixed(2) || '0.00'}`;
                    itemsHtml += `
                        <div class="purchased-item">
                            <img src="${item.image || '/placeholder-image.png'}" alt="${item.title}">
                            <div class="item-info">
                                <p class="item-title">${item.title}</p>
                                <p class="item-qty-price">Qty: ${item.quantity} | ${itemPriceFormatted}</p>
                            </div>
                        </div>
                    `;
                 });
                 itemsContainer.innerHTML = itemsHtml;
            } else {
                itemsContainer.innerHTML = '<p>No items found for this order.</p>';
            }
        }

        // --- Render Shipping Address ---
        if (shippingContainer) {
            if (shippingInfo) {
                shippingContainer.innerHTML = `
                    <p class="mb-1 font-medium">${shippingInfo.name} ${shippingInfo.lastname}</p>
                    <p class="text-sm text-neutral-600">${shippingInfo.street}</p>
                    <p class="text-sm text-neutral-600">${shippingInfo.city}, ${shippingInfo.postcode}</p>
                    <p class="text-sm text-neutral-600">${shippingInfo.country}</p>
                    <p class="text-sm text-neutral-600 mt-2">Phone: ${shippingInfo.phone}</p>
                    <p class="text-sm text-neutral-600">Email: ${shippingInfo.email}</p>
                    ${shippingInfo.orderNote ? `<p class="text-sm text-neutral-600 mt-2"><i>Note: ${shippingInfo.orderNote}</i></p>` : ''}
                `;
            } else {
                shippingContainer.innerHTML = '<p>Shipping address not available.</p>';
            }
        }

    } catch (error) {
        console.error("Error loading order confirmation:", error);
        if (successMsgContainer) { // Use success container for general errors too
            successMsgContainer.innerHTML = `
                <p class="text-red-600">Error</p>
                <span class="title block text-red-700">Could not load order details.</span>
                <p class="arrival-info mt-2">Order ID: ${orderId || 'Unknown'}. Please contact support if this issue persists.</p>
            `;
        }
         // Clear other sections
        if(detailsContainer) detailsContainer.innerHTML = '';
        if(itemsContainer) itemsContainer.innerHTML = '';
        if(shippingContainer) shippingContainer.innerHTML = '';
    }
}

// --- Add Simulated API function (needs to be moved to api.js) ---
// This is just a placeholder - move to api.js
async function fetchOrderDetails(orderId) {
    console.log(`API: Simulating Fetch Order Details for ${orderId}`);
    await new Promise(resolve => setTimeout(resolve, 220));

    // Retrieve stored cart items if possible (this relies on billing.js not clearing too early, or storing it temporarily)
    // OR reconstruct based on orderId
    // Let's simulate reconstruction based on ID + some static data
    const allBilling = JSON.parse(localStorage.getItem('veducationBilling') || '[]');
    const billingInfo = allBilling.find(b => b.id == orderId)?.attributes; // Use == for potential string/number mismatch

    // Simulate items based on typical orders or store the cart temporarily
    // For now, using items from the general simulated data
    const simulatedItems = [
        SIMULATED_DATA.categories.store.products[0], // Tshirt
        SIMULATED_DATA.categories.books.products[0]  // Book
    ].map(p => ({ // Convert to cart item format
        id: p.id,
        title: p.attributes.title,
        price: p.attributes.price,
        image: p.attributes.posterImageUrl?.data?.attributes?.url || '/placeholder-image.png',
        quantity: Math.floor(Math.random() * 2) + 1 // Random quantity 1 or 2
    }));


    if (!billingInfo) {
        console.warn(`Billing info not found for simulated order ${orderId}`);
        // return null; // Or return partial data
    }

    return {
        orderInfo: {
            id: orderId,
            date: Date.now(), // Use current time for simulation
            total: simulatedItems.reduce((sum, item) => sum + (item.price * item.quantity), 0) * 1.1, // Crude total with tax
            paymentMethod: 'Card (Simulated)'
        },
        items: simulatedItems,
        shippingInfo: billingInfo || { name: 'N/A', street: 'N/A', city: 'N/A', postcode: 'N/A', country: 'N/A', phone: 'N/A', email: 'N/A'} // Provide fallback
    };
}


// --- Initialization ---
document.addEventListener('DOMContentLoaded', loadOrderConfirmation);
