// script.js
// Get the form element by its id
const form = document.getElementById("myForm");
// Get the messages element by its id
const messages = document.getElementById("messages");

let lastMessage = null; // variable to store the last message

// Add an async keyword to your event handler function
form.addEventListener("submit", async function(event) {
  // Prevent the default behavior of the browser
  event.preventDefault();
  // Create a FormData object from the form element
  const formData = new FormData(form);
  // Create an object that contains the name and message fields from the form data
  const data = {
    name: formData.get("name"),
    message: formData.get("message")
  };
  // Convert the object to a JSON string
  const json = JSON.stringify(data);

  form.message.value = '';


  try {
    // Use await to wait for the fetch function to complete
    const response = await fetch("messages.json", {
      method: "POST",
      body: json,
      headers: {
        "Content-Type": "application/json"
      }
    });

    // Make a GET request to fetch the updated data
    const getResponse = await fetch("messages.json");
    let getData = await getResponse.json();

    // Convert getData to an array if it's an object
    if (typeof getData === 'object' && !Array.isArray(getData)) {
      getData = Object.values(getData);
    }

    // Do something with the updated data from the server
    console.log(getData);

    // Clear the messages element
    messages.innerHTML = '';

    // Iterate over each item in the array
    let isNewMessage = false;
    getData.forEach(item => {
      // You can also display the updated data on the page
      // For example, create a message element and append it to the messages element
      const message = document.createElement("div");
      message.className = "message";
      message.innerHTML = `
        <div class="message-content">
          <span class="message-name">${item.name}</span>
          <span class="message-text">${item.message}</span>
        </div>
      `;
      messages.appendChild(message);

      // Check if there's a new message
      if (lastMessage === null || item.message !== lastMessage.message) {
        isNewMessage = true;
      }

      // Update the last message
      lastMessage = item;
    });

    // Scroll to the bottom of the messages element if there's a new message
    if (isNewMessage) {
      messages.scrollTop = messages.scrollHeight;
    }
  } catch (error) {
    // Handle any errors
    console.error(error);
  }
});

// Fetch messages every second
setInterval(async () => {
  const response = await fetch("messages.json");
  let data = await response.json();

  // Convert data to an array if it's an object
  if (typeof data === 'object' && !Array.isArray(data)) {
    data = Object.values(data);
  }

  // Clear the messages element
  messages.innerHTML = '';

  // Iterate over each item in the array
  let isNewMessage = false;
  data.forEach(item => {
    // Display the data on the page
    const message = document.createElement("div");
    message.className = "message";
    message.innerHTML = `
      <div class="message-content">
        <span class="message-name">${item.name}</span>
        <span class="message-text">${item.message}</span>
      </div>
    `;
    messages.appendChild(message);

    // Check if there's a new message
    if (lastMessage === null || item.message !== lastMessage.message) {
      isNewMessage = true;
    }

    // Update the last message
    lastMessage = item;
  });

  // Scroll to the bottom of the messages element if there's a new message
  if (isNewMessage) {
    messages.scrollTop = messages.scrollHeight;
  }
}, 1000);
