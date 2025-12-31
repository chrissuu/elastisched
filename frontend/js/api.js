import { API_BASE, demoBlobs, state } from "./core.js";
import { renderAll, setActive } from "./render.js";

async function fetchBlobs() {
  try {
    const response = await fetch(`${API_BASE}/blobs`);
    if (!response.ok) {
      throw new Error("Failed to fetch blobs");
    }
    const data = await response.json();
    state.blobs = data;
  } catch (error) {
    state.blobs = demoBlobs;
  }
  renderAll();
  setActive(state.view);
}

export { fetchBlobs };
