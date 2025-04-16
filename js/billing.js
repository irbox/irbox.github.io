// --- Billing Details Form Logic ---

/**
 * Validates the billing form data.
 * @param {object} formData - Object containing form field values.
 * @returns {object} An errors object. Empty if no errors, otherwise keys match field names.
 */
function validateBillingForm(formData) {
    const errors = {};
    if (!formData.name?.trim()) errors.name = 'First name is required.';
    if (!formData.lastname?.trim()) errors.lastname = 'Last name is required.';
    if (!formData.email?.trim()) {
        errors.email = 'Email is required.';
    } else if (!/\S+@\S+\.\S+/.test(formData.email)) { // Basic email format check
        errors.email = 'Email address is invalid.';
    }
    if (!formData.phone?.trim()) errors.phone = 'Phone number is required.';
    // Add more specific phone validation if needed (e.g., regex for digits/format)
    if (!formData.street?.trim()) errors.street = 'Street address is required.';
    if (!formData.city?.trim()) errors.city = 'City is required.';
    if (!formData.postcode?.trim()) errors.postcode = 'Postcode / ZIP is required.';
    if (!formData.country?.trim()) errors.country = 'Country is required.';

    // Add more validation rules as needed

    return errors;
}

/**
 * Displays validation errors on the form.
 * @param {object} errors - The errors object from validateBillingForm.
 */
function displayBillingErrors(errors) {
    // Clear previous errors
    $$('#billing-form .field-error').forEach(el => el.textContent = '');
    $('#server-error').textContent = ''; // Clear server errors too

    for (const field in errors) {
        const errorElement = $(`#billing-form .field-error[data-field="${field}"]`);
        if (errorElement) {
            errorElement.textContent = errors[field];
        }
        // Optionally highlight the input field itself
        $(`#billing-form [name="${field}"]`)?.classList.add('border-red-500');
    }
}

/**
 * Clears validation error styles and messages.
 */
function clearBillingErrors() {
     $$('#billing-form .field-error').forEach(el => el.textContent = '');
     $('#server-error').textContent = '';
     // Remove red borders
     $$('#billing-form input, #billing-form select, #billing-form textarea').forEach(el => el.classList.remove('border-red-500'));
}

/**
 * Handles the billing form submission.
 * @param {Event} event - The form submit event.
 */
async function handleBillingSubmit(event) {
    event.preventDefault(); // Prevent actual form submission
    console.log("Handling billing form submission...");

    clearBillingErrors();
    const form = event.target;
    const submitButton = $('#submit-billing-btn');
    const loadingIndicator = $('#billing-loading');
    const serverErrorElement = $('#server-error');

    // Show loading state (optional)
    if (submitButton) submitButton.disabled = true;
    if (loadingIndicator) loadingIndicator.classList.remove('hidden');
    if (serverErrorElement) serverErrorElement.textContent = '';


    // Collect form data
    const formData = {
        name: form.elements.name.value,
        lastname: form.elements.lastname.value,
        email: form.elements.email.value,
        phone: form.elements.phone.value,
        country: form.elements.country.value,
        street: form.elements.street.value,
        city: form.elements.city.value,
        postcode: form.elements.postcode.value,
        orderNote: form.elements.orderNote.value,
    };

    // Validate form data
    const validationErrors = validateBillingForm(formData);

    if (Object.keys(validationErrors).length > 0) {
        console.log("Validation failed:", validationErrors);
        displayBillingErrors(validationErrors);
        if (submitButton) submitButton.disabled = false;
        if (loadingIndicator) loadingIndicator.classList.add('hidden');
        return; // Stop submission if errors exist
    }

    console.log("Validation passed. Submitting data (simulated):", formData);

    // Get user ID and cart items from URL (needed for API call and redirect)
    const user = getCurrentUser(); // From auth.js
     const cartItemIds = getQueryParam('cartItems'); // From utils.js

    if (!user) {
        console.error("User not logged in. Cannot submit billing details.");
        if (serverErrorElement) serverErrorElement.textContent = 'You must be logged in to save details.';
        if (submitButton) submitButton.disabled = false;
        if (loadingIndicator) loadingIndicator.classList.add('hidden');
        // Optionally redirect to login or show login prompt
        return;
    }

    const apiPayload = {
        ...formData,
        user_id: user.id // Add user ID expected by the (simulated) API
    };


    try {
        // Call the (simulated) API function
        const response = await addBillingDetails(apiPayload); // From api.js

        if (response.success) {
            console.log("Billing details saved successfully (simulated).");

            // Clear cart from localStorage (assuming purchase is now 'complete')
            // Note: Original code did this *after* API call in BillingDetails container.
            // Be mindful if the order confirmation page *needs* cart data still.
            // If order confirm page re-fetches order, clearing here is fine.
            const cart = getCart(); // Need cart details if emptyCart needs product IDs/quantities
            clearCart(); // From app.js - clears localStorage and updates badge

            // Redirect to order confirmation page, passing along the cart item IDs (or a new order ID from response)
            window.location.href = `/orderconfirmed.html?orderId=${response.data.billingId}&status=success`; // Example redirect

        } else {
            console.error("Failed to save billing details (simulated):", response.message);
             if (serverErrorElement) serverErrorElement.textContent = response.message || 'An unknown error occurred.';
        }

    } catch (error) {
        console.error("Error submitting billing details:", error);
        if (serverErrorElement) serverErrorElement.textContent = 'An error occurred while submitting your details. Please try again.';
    } finally {
        // Hide loading state
         if (submitButton) submitButton.disabled = false;
         if (loadingIndicator) loadingIndicator.classList.add('hidden');
    }
}

// --- Initialization ---
// Add event listener when the DOM is ready and if the form exists
document.addEventListener('DOMContentLoaded', () => {
    const billingForm = document.getElementById('billing-form');
    if (billingForm) {
        // Check if user is logged in, maybe pre-fill email if available
        const user = getCurrentUser();
        if (user && form.elements.email) {
            form.elements.email.value = user.email || '';
             // Could pre-fill other fields if they exist in user object
             // form.elements.name.value = user.name?.split(' ')[0] || '';
             // form.elements.lastname.value = user.name?.split(' ')[1] || '';
        }

        billingForm.addEventListener('submit', handleBillingSubmit);
    } else {
        // Only log if we expect the form on this page (i.e., we are on billingdetails.html)
        if (window.location.pathname.includes('billingdetails.html')) {
            console.warn('Billing form not found on this page.');
        }
    }
});
