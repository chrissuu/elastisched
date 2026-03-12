const DEFAULT_REDIRECT = "/ui";
const statusNode = document.getElementById("authStatus");
const loginForm = document.getElementById("loginForm");
const registerForm = document.getElementById("registerForm");
const tabButtons = Array.from(document.querySelectorAll(".auth-tab"));
const formsByTab = {
  login: loginForm,
  register: registerForm,
};

function nextPath() {
  const params = new URLSearchParams(window.location.search);
  const raw = params.get("next");
  if (!raw || !raw.startsWith("/")) {
    return DEFAULT_REDIRECT;
  }
  if (raw.startsWith("//")) {
    return DEFAULT_REDIRECT;
  }
  return raw;
}

function setStatus(message = "", kind = "") {
  statusNode.textContent = message;
  statusNode.classList.remove("error", "success");
  if (kind) {
    statusNode.classList.add(kind);
  }
}

function setActiveTab(tabName) {
  tabButtons.forEach((button) => {
    const active = button.dataset.tabTarget === tabName;
    button.classList.toggle("active", active);
  });
  Object.entries(formsByTab).forEach(([name, form]) => {
    form.classList.toggle("active", name === tabName);
  });
  setStatus("");
}

function parseErrorPayload(payload) {
  if (!payload) return "Request failed.";
  if (typeof payload.detail === "string") return payload.detail;
  if (Array.isArray(payload.detail)) {
    return payload.detail.map((item) => item?.msg).filter(Boolean).join("; ") || "Request failed.";
  }
  return "Request failed.";
}

async function submitAuth(path, payload, submitButton) {
  submitButton.disabled = true;
  try {
    const response = await fetch(path, {
      method: "POST",
      credentials: "include",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });
    if (!response.ok) {
      const data = await response.json().catch(() => null);
      throw new Error(parseErrorPayload(data));
    }
    const data = await response.json();
    if (data?.csrf_token) {
      window.localStorage.setItem("elastisched:csrf", data.csrf_token);
    }
    setStatus("Success. Redirecting to your workspace...", "success");
    window.location.assign(nextPath());
  } catch (error) {
    setStatus(error?.message || "Request failed.", "error");
  } finally {
    submitButton.disabled = false;
  }
}

async function tryResumeSession() {
  try {
    const response = await fetch("/auth/me", {
      credentials: "include",
      headers: { Accept: "application/json" },
    });
    if (!response.ok) return;
    const data = await response.json();
    if (data?.csrf_token) {
      window.localStorage.setItem("elastisched:csrf", data.csrf_token);
    }
    window.location.assign(nextPath());
  } catch (_error) {
    // Keep landing page visible.
  }
}

tabButtons.forEach((button) => {
  button.addEventListener("click", () => {
    setActiveTab(button.dataset.tabTarget || "login");
  });
});

loginForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  const submitButton = loginForm.querySelector("button[type='submit']");
  const formData = new FormData(loginForm);
  await submitAuth(
    "/auth/login",
    {
      email: String(formData.get("email") || ""),
      password: String(formData.get("password") || ""),
    },
    submitButton
  );
});

registerForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  const submitButton = registerForm.querySelector("button[type='submit']");
  const formData = new FormData(registerForm);
  await submitAuth(
    "/auth/register",
    {
      email: String(formData.get("email") || ""),
      password: String(formData.get("password") || ""),
      display_name: String(formData.get("display_name") || "").trim() || null,
    },
    submitButton
  );
});

setActiveTab("login");
tryResumeSession();
