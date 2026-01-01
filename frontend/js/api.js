import { API_BASE, appConfig, state } from "./core.js";
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
    state.blobs = [];
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

async function fetchScheduleStatus() {
  const response = await fetch(`${API_BASE}/schedule/status`);
  if (!response.ok) {
    throw new Error("Failed to fetch schedule status");
  }
  return response.json();
}

async function runSchedule(granularityMinutes, lookaheadSeconds) {
  const payload = {
    granularity_minutes: granularityMinutes,
    lookahead_seconds: lookaheadSeconds,
  };
  const response = await fetch(`${API_BASE}/schedule`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });
  if (!response.ok) {
    let detail = "Failed to run scheduler";
    const contentType = response.headers.get("content-type") || "";
    if (contentType.includes("application/json")) {
      const data = await response.json();
      detail = data.detail || detail;
    } else {
      detail = (await response.text()) || detail;
    }
    throw new Error(detail);
  }
  return response.json();
}

export { ensureOccurrences, fetchOccurrences, fetchScheduleStatus, runSchedule };
