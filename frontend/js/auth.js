const SAFE_METHODS = new Set(["GET", "HEAD", "OPTIONS"]);
const CSRF_STORAGE_KEY = "elastisched:csrf";

const authState = {
  user: null,
  csrfToken: null,
};

function getStoredCsrfToken() {
  try {
    return window.localStorage.getItem(CSRF_STORAGE_KEY);
  } catch (_error) {
    return null;
  }
}

function storeCsrfToken(token) {
  authState.csrfToken = token || null;
  try {
    if (token) {
      window.localStorage.setItem(CSRF_STORAGE_KEY, token);
    } else {
      window.localStorage.removeItem(CSRF_STORAGE_KEY);
    }
  } catch (_error) {
    // Ignore storage errors.
  }
}

async function fetchSession() {
  const response = await fetch("/auth/me", {
    method: "GET",
    credentials: "include",
    headers: {
      Accept: "application/json",
    },
  });
  if (!response.ok) {
    storeCsrfToken(null);
    authState.user = null;
    return null;
  }
  const payload = await response.json();
  authState.user = payload?.user || null;
  storeCsrfToken(payload?.csrf_token || null);
  return payload;
}

function redirectToLanding() {
  const current =
    `${window.location.pathname || "/ui"}${window.location.search || ""}${window.location.hash || ""}`;
  const next = encodeURIComponent(current);
  window.location.assign(`/?next=${next}`);
}

export async function requireAuthenticatedAppSession() {
  const session = await fetchSession();
  if (!session) {
    redirectToLanding();
    throw new Error("Not authenticated");
  }
  return session;
}

async function getCsrfToken() {
  if (authState.csrfToken) {
    return authState.csrfToken;
  }
  const stored = getStoredCsrfToken();
  if (stored) {
    authState.csrfToken = stored;
    return stored;
  }
  const session = await fetchSession();
  return session?.csrf_token || null;
}

async function executeRequest(input, init = {}, retry = false) {
  const method = String(init.method || "GET").toUpperCase();
  const headers = new Headers(init.headers || {});
  if (!headers.has("Accept")) {
    headers.set("Accept", "application/json");
  }
  if (!SAFE_METHODS.has(method)) {
    const csrfToken = await getCsrfToken();
    if (csrfToken && !headers.has("X-CSRF-Token")) {
      headers.set("X-CSRF-Token", csrfToken);
    }
  }
  const response = await fetch(input, {
    ...init,
    method,
    credentials: "include",
    headers,
  });
  if (
    !retry &&
    response.status === 403 &&
    !SAFE_METHODS.has(method) &&
    (await response.clone().text()).toLowerCase().includes("csrf")
  ) {
    await fetchSession();
    return executeRequest(input, init, true);
  }
  if (response.status === 401) {
    redirectToLanding();
  }
  return response;
}

export async function apiFetch(input, init = {}) {
  return executeRequest(input, init);
}

export function getAuthenticatedUser() {
  return authState.user;
}

export function getCachedCsrfToken() {
  return authState.csrfToken || getStoredCsrfToken();
}

export async function logoutAndRedirect() {
  const csrfToken = await getCsrfToken();
  const headers = new Headers();
  if (csrfToken) {
    headers.set("X-CSRF-Token", csrfToken);
  }
  await fetch("/auth/logout", {
    method: "POST",
    credentials: "include",
    headers,
  }).catch(() => {});
  storeCsrfToken(null);
  window.location.assign("/");
}
