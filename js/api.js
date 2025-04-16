// --- API Data Fetching Functions (Simulated) ---

// Base URL (If you were fetching from a real API)
// const API_BASE_URL = 'https://your-api.com/api';

// --- Simulated Data (Replace with actual static data or fetch calls if applicable) ---

const SIMULATED_DATA = {
  home: {
    bannerTitle: "Unlock Ancient Wisdom",
    // Add other home-specific data if needed
  },
  about: {
    title: "Prateeik Prajapati",
    greet: "Learn about Vedic science, history, philosophy & culture in detail.",
    desc: "With Prateeik Prajapati, who has been reading and researching collective psychologies of world civilisations since last 7 years.\n\nNow he has decided to share all that invaluable knowledge with the world through his talks and sessions in schools, colleges, corporates, villages & online through Youtube & social media.",
    img: "/AboutPageImg.png", // Path relative to public folder (or root for gh-pages)
  },
  categories: {
    books: {
      id: 'books',
      name: 'Featured Books',
      products: [
        { id: 1, attributes: { title: "Sanatan Sanskriti ka Moolgyan", author: "Prateeik Prajapati", price: 15.00, posterImageUrl: { data: { attributes: { url: '/book_sanatan_sanskriti.png' } } }, description: "Deep dive into the fundamentals of Sanatan Dharma." } },
        { id: 2, attributes: { title: "Basics of Sanatan Sanskriti", author: "Prateeik Prajapati", price: 12.00, posterImageUrl: { data: { attributes: { url: '/book_basics_sanatan.png' } } }, description: "An introductory guide to Sanatan Culture." } },
        { id: 3, attributes: { title: "New Yorkers Short Stories", author: "O. Henry", price: 9.99, posterImageUrl: { data: { attributes: { url: '/book_new_yorkers.jpg' } } }, description: "Classic short stories about life in New York." } },
        { id: 4, attributes: { title: "Learn Javascript with Ease", author: "Stephen Blumenthal", price: 25.00, posterImageUrl: { data: { attributes: { url: '/book_javascript.jpg' } } }, description: "From beginner to expert in less than a week." } },
        // Add more book products
      ]
    },
    store: {
      id: 'store',
      name: 'Yoddha Store',
      products: [
        { id: 101, attributes: { title: "Veducation T-Shirt", price: 20.00, posterImageUrl: { data: { attributes: { url: '/tshirt_black.png' } } }, description: "Pure black Veducation T-Shirt for Men." } },
        { id: 102, attributes: { title: "Wooden Indian Clubs (Mugdar)", price: 45.00, posterImageUrl: { data: { attributes: { url: '/store_mugdar.png' } } }, description: "Traditional Indian clubs for exercise." } },
        { id: 103, attributes: { title: "Tulsi Mala", price: 10.00, posterImageUrl: { data: { attributes: { url: '/store_mala.png' } } }, description: "Authentic Tulsi prayer beads." } },
        // Add more store products
      ]
    }
  },
  courses: [
      { id: 201, attributes: { title: "Scientific Evidence of Soul", description: "Exploring the scientific perspectives on the concept of the soul. Veducation S01E01.", price: 49.99, img: '/course_soul.png', videoLink: '/videos/sample_video.mp4', lessons: [ {id: 'v1', title: 'Lesson 1: Intro', link: '/videos/sample_video.mp4', thumbnail:'/course_soul.png', time: '10:00', description: 'Introduction to the course.'} /* ... more lessons */ ] } },
      { id: 202, attributes: { title: "The Concept of Dharma", description: "Understanding Dharma in the context of Vedic scriptures.", price: 39.99, img: '/course_placeholder_2.png', videoLink: '/videos/sample_video.mp4', lessons: [ /* ... lessons */ ] } },
      // Add more courses
  ],
  donations: [
      { id: 301, attributes: { title: "Building Gaushala for Gaumatas", raised: "50568", raisedOutOf: "100000", daysRemaining: 50, backers: 24000, img: '/img2.png' } },
      { id: 302, attributes: { title: "Spreading Vedic Knowledge Initiative", raised: "15200", raisedOutOf: "50000", daysRemaining: 84, backers: 1000, img: '/img4.png' } },
      // Add more donation campaigns
  ],
  users: [ // Simulate user data store if needed (e.g., for profiles) - NOT secure
    { id: 1, email: 'static-user@example.com', name: 'Static User', address: '123 Main St', city: 'Anytown', postcode: '12345', country: 'USA', phone: '555-1234' }
  ]
  // Add more top-level data keys as needed (e.g., library books, notifications)
};


// --- Data Fetching Function Implementations (Simulated) ---

async function fetchHomeData() {
  console.log("API: Fetching Home Data (Simulated)");
  // Simulate network delay
  await new Promise(resolve => setTimeout(resolve, 150));
  return SIMULATED_DATA.home;
}

async function fetchAboutData() {
  console.log("API: Fetching About Data (Simulated)");
  await new Promise(resolve => setTimeout(resolve, 100));
  // Directly return the structured data including description and image path
  return SIMULATED_DATA.about;
}

async function fetchCategoryData(categoryId) {
  console.log(`API: Fetching Category Data for ${categoryId} (Simulated)`);
  await new Promise(resolve => setTimeout(resolve, 200));
  const category = SIMULATED_DATA.categories[categoryId];
  if (category) {
    // Simulate the structure expected by renderProductList
    return {
        name: category.name,
        products: category.products // Assuming products are already in the correct array format
    }
  } else {
      console.warn(`Category "${categoryId}" not found in simulated data.`);
      return { name: `Category: ${categoryId}`, products: [] }; // Return empty structure
  }
}


async function fetchProductDetails(productId) {
    console.log(`API: Fetching Product Details for ${productId} (Simulated)`);
    await new Promise(resolve => setTimeout(resolve, 180));

    // Search across all categories for the product
    let foundProduct = null;
    for (const categoryKey in SIMULATED_DATA.categories) {
        const category = SIMULATED_DATA.categories[categoryKey];
        foundProduct = category.products.find(p => p.id == productId); // Use == for potential string/number mismatch
        if (foundProduct) break;
    }

    if (foundProduct) {
         // Simulate the structure including nested attributes for image URL
         return {
             id: foundProduct.id,
             attributes: {
                ...foundProduct.attributes // Spread existing attributes
             }
         };
    } else {
        console.warn(`Product with ID "${productId}" not found.`);
        return null; // Indicate not found
    }
}


async function fetchCoursesData() {
  console.log("API: Fetching Courses Data (Simulated)");
  await new Promise(resolve => setTimeout(resolve, 250));
  return SIMULATED_DATA.courses; // Return the array directly
}

async function fetchCourseDetails(courseId) {
    console.log(`API: Fetching Course Details for ${courseId} (Simulated)`);
    await new Promise(resolve => setTimeout(resolve, 190));
    const course = SIMULATED_DATA.courses.find(c => c.id == courseId); // Use == for potential mismatch
     if (course) {
         // Simulate the structure including nested attributes
         return {
             id: course.id,
             attributes: {
                 ...course.attributes
             }
         };
     } else {
         console.warn(`Course with ID "${courseId}" not found.`);
         return null;
     }
}


async function fetchDonationsData() {
  console.log("API: Fetching Donations Data (Simulated)");
  await new Promise(resolve => setTimeout(resolve, 120));
  return SIMULATED_DATA.donations; // Return the array directly
}

// --- Simulated Action Functions (No real backend interaction) ---

async function addPurchase(purchaseData) {
    console.log("API: Simulating Add Purchase:", purchaseData);
    // In a real app, this would POST to your server
    await new Promise(resolve => setTimeout(resolve, 300));
    // Simulate success
    return { success: true, message: "Purchase simulated successfully.", data: { purchaseId: Date.now() } }; // Return a dummy ID
}

async function addBillingDetails(billingData) {
    console.log("API: Simulating Add Billing Details:", billingData);
    // In a real app, this would POST to your server
    await new Promise(resolve => setTimeout(resolve, 280));
     // Simulate success - maybe store locally for demo if needed on order confirm page
     const existingBilling = JSON.parse(localStorage.getItem('veducationBilling') || '[]');
     existingBilling.push({ id: Date.now(), attributes: billingData });
     localStorage.setItem('veducationBilling', JSON.stringify(existingBilling));
    return { success: true, message: "Billing details simulated successfully.", data: { billingId: Date.now() } };
}

async function fetchBillingDetails(userId) {
    console.log(`API: Simulating Fetch Billing Details for user ${userId}`);
    await new Promise(resolve => setTimeout(resolve, 50));
    const allBilling = JSON.parse(localStorage.getItem('veducationBilling') || '[]');
    // Find the specific user's billing (This is crude, real app uses backend lookup)
    const userBilling = allBilling.find(b => b.attributes.user_id == userId);
    return userBilling ? [userBilling] : []; // Return array structure like original?
}


async function validatePromoCode(code) {
    console.log(`API: Simulating Validate Promo Code: ${code}`);
    await new Promise(resolve => setTimeout(resolve, 80));
    const validCodes = {
        "DISCOUNT10": { rate: "percentage", amount: 10 }, // 10% off
        "SAVE5": { rate: "fixed", amount: 5 } // $5 off
    };
    if (validCodes[code]) {
        return { success: true, codeData: validCodes[code] };
    } else {
        return { success: false, message: "Invalid promo code" };
    }
}

// Add more simulated fetch/action functions as needed based on the original app's API calls
// e.g., fetchLibraryBooks, submitReview, fetchNotifications, etc.
