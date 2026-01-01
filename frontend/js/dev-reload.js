const isLocalHost =
  window.location.hostname === "localhost" || window.location.hostname === "127.0.0.1";

if (isLocalHost) {
  const paths = [
    "./index.html",
    "./css/styles.css",
    "./js/app.js",
    "./js/core.js",
    "./js/dom.js",
    "./js/forms.js",
    "./js/render.js",
    "./js/utils.js",
    "./js/constants.js",
  ];

  const signatures = new Map();
  let checking = false;

  async function fetchSignature(path) {
    const url = `${path}?_reload=${Date.now()}`;
    try {
      const head = await fetch(url, { method: "HEAD", cache: "no-store" });
      if (!head.ok) return null;
      const sig =
        head.headers.get("etag") ||
        head.headers.get("last-modified") ||
        head.headers.get("content-length");
      if (sig) return sig;
    } catch (error) {
      // Fallback to GET below.
    }

    try {
      const response = await fetch(url, { cache: "no-store" });
      if (!response.ok) return null;
      const text = await response.text();
      return String(text.length);
    } catch (error) {
      return null;
    }
  }

  async function checkForUpdates() {
    if (checking) return;
    checking = true;
    try {
      for (const path of paths) {
        const signature = await fetchSignature(path);
        if (!signature) continue;
        const previous = signatures.get(path);
        if (previous && previous !== signature) {
          window.location.reload();
          return;
        }
        signatures.set(path, signature);
      }
    } finally {
      checking = false;
    }
  }

  setInterval(checkForUpdates, 1500);
  checkForUpdates();
}
