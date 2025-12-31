import { API_BASE, appConfig, demoBlobs, state } from "./core.js";
import { toProjectIsoFromDate } from "./utils.js";

async function fetchOccurrences(start, end) {
  try {
    const query = new URLSearchParams({
      start: toProjectIsoFromDate(start, appConfig.projectTimeZone),
      end: toProjectIsoFromDate(end, appConfig.projectTimeZone),
    });
    const response = await fetch(`${API_BASE}/occurrences?${query.toString()}`);
    if (!response.ok) {
      throw new Error("Failed to fetch occurrences");
    }
    const data = await response.json();
    state.blobs = data;
    state.loadedRange = { start, end };
  } catch (error) {
    state.blobs = demoBlobs;
    state.loadedRange = null;
  }
}

function rangeCovers(loadedRange, start, end) {
  if (!loadedRange) return false;
  return start >= loadedRange.start && end <= loadedRange.end;
}

async function ensureOccurrences(start, end) {
  if (rangeCovers(state.loadedRange, start, end)) {
    return;
  }
  await fetchOccurrences(start, end);
}

export { ensureOccurrences, fetchOccurrences };
