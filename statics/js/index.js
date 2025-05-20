const menuIcon = document.getElementById("menu-icon");
const sidebar = document.getElementById("sidebar");

menuIcon.addEventListener("click", () => {
  sidebar.classList.toggle("collapsed");
});

async function standardizeText() {
  const inputText = document.getElementById("inputText").value;
  const outputTextElement = document.getElementById("outputText");
  const loadingSpinner = document.getElementById("loading");

  outputTextElement.textContent = "";
  loadingSpinner.style.display = "inline-block";

  try {
    const response = await fetch("http://localhost:8000/correct", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ text: inputText }),
    });

    const data = await response.json();
    outputTextElement.textContent =
      data.corrected_text || "Không có kết quả.";
  } catch (error) {
    outputTextElement.textContent =
      "❌ Lỗi: Không thể kết nối đến server.";
  } finally {
    loadingSpinner.style.display = "none";
  }
}