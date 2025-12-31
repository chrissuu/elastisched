import { minuteGranularity } from "./core.js";

function toDate(value) {
  return value ? new Date(value) : null;
}

function getLocalTimeZone() {
  return Intl.DateTimeFormat().resolvedOptions().timeZone || "UTC";
}

function formatOffset(minutes) {
  const sign = minutes <= 0 ? "+" : "-";
  const abs = Math.abs(minutes);
  const hours = `${Math.floor(abs / 60)}`.padStart(2, "0");
  const mins = `${abs % 60}`.padStart(2, "0");
  return `${sign}${hours}:${mins}`;
}

function toIso(value) {
  const date = new Date(value);
  if (Number.isNaN(date.getTime())) return "";
  const year = date.getFullYear();
  const month = `${date.getMonth() + 1}`.padStart(2, "0");
  const day = `${date.getDate()}`.padStart(2, "0");
  const hours = `${date.getHours()}`.padStart(2, "0");
  const minutes = `${date.getMinutes()}`.padStart(2, "0");
  const seconds = `${date.getSeconds()}`.padStart(2, "0");
  const offset = formatOffset(date.getTimezoneOffset());
  return `${year}-${month}-${day}T${hours}:${minutes}:${seconds}${offset}`;
}

function toLocalInputValue(isoString) {
  const date = toDate(isoString);
  if (!date) return "";
  const year = date.getFullYear();
  const month = `${date.getMonth() + 1}`.padStart(2, "0");
  const day = `${date.getDate()}`.padStart(2, "0");
  const hours = `${date.getHours()}`.padStart(2, "0");
  const minutes = `${date.getMinutes()}`.padStart(2, "0");
  return `${year}-${month}-${day}T${hours}:${minutes}`;
}

function toLocalInputFromDate(date) {
  if (!date) return "";
  const year = date.getFullYear();
  const month = `${date.getMonth() + 1}`.padStart(2, "0");
  const day = `${date.getDate()}`.padStart(2, "0");
  const hours = `${date.getHours()}`.padStart(2, "0");
  const minutes = `${date.getMinutes()}`.padStart(2, "0");
  return `${year}-${month}-${day}T${hours}:${minutes}`;
}

function clampToGranularity(minutes) {
  return Math.round(minutes / minuteGranularity) * minuteGranularity;
}

function pad(num) {
  return num.toString().padStart(2, "0");
}

function formatTimeRange(start, end) {
  const startDate = toDate(start);
  const endDate = toDate(end);
  if (!startDate || !endDate) {
    return "";
  }
  const startLabel = `${pad(startDate.getHours())}:${pad(startDate.getMinutes())}`;
  const endLabel = `${pad(endDate.getHours())}:${pad(endDate.getMinutes())}`;
  return `${startLabel} - ${endLabel}`;
}

function startOfDay(date) {
  const copy = new Date(date);
  copy.setHours(0, 0, 0, 0);
  return copy;
}

function addDays(date, count) {
  const copy = new Date(date);
  copy.setDate(copy.getDate() + count);
  return copy;
}

function getWeekStart(date) {
  const dayOfWeek = date.getDay();
  return addDays(startOfDay(date), dayOfWeek === 0 ? -6 : 1 - dayOfWeek);
}

function getViewRange(view, anchorDate) {
  if (view === "day") {
    const start = startOfDay(anchorDate);
    return { start, end: addDays(start, 1) };
  }
  if (view === "week") {
    const dayOfWeek = anchorDate.getDay();
    const monday = addDays(anchorDate, dayOfWeek === 0 ? -6 : 1 - dayOfWeek);
    const start = startOfDay(monday);
    return { start, end: addDays(start, 7) };
  }
  if (view === "month") {
    const start = new Date(anchorDate.getFullYear(), anchorDate.getMonth(), 1);
    const end = new Date(anchorDate.getFullYear(), anchorDate.getMonth() + 1, 1);
    return { start, end };
  }
  const start = new Date(anchorDate.getFullYear(), 0, 1);
  const end = new Date(anchorDate.getFullYear() + 1, 0, 1);
  return { start, end };
}

function getTagType(tags) {
  if (tags?.includes("deep")) return "deep";
  if (tags?.includes("admin")) return "admin";
  return "focus";
}

function overlaps(rangeStart, rangeEnd, eventStart, eventEnd) {
  return eventStart < rangeEnd && eventEnd > rangeStart;
}

function layoutBlocks(blocks) {
  const sorted = [...blocks].sort((a, b) => a.startMin - b.startMin || a.endMin - b.endMin);
  const active = [];
  let cluster = null;
  let clusterId = 0;

  sorted.forEach((block) => {
    for (let i = active.length - 1; i >= 0; i -= 1) {
      if (active[i].endMin <= block.startMin) {
        active.splice(i, 1);
      }
    }

    if (active.length === 0) {
      clusterId += 1;
      cluster = { id: clusterId, maxColumns: 0, events: [] };
    }

    const used = new Set(active.map((item) => item.column));
    let column = 0;
    while (used.has(column)) {
      column += 1;
    }

    block.column = column;
    block.cluster = cluster;
    cluster.events.push(block);
    active.push(block);

    const activeColumns = new Set(active.map((item) => item.column));
    cluster.maxColumns = Math.max(cluster.maxColumns, activeColumns.size);
  });

  blocks.forEach((block) => {
    block.columns = block.cluster?.maxColumns || 1;
  });
}

export {
  addDays,
  clampToGranularity,
  formatTimeRange,
  getTagType,
  getViewRange,
  getWeekStart,
  getLocalTimeZone,
  layoutBlocks,
  overlaps,
  startOfDay,
  toDate,
  toIso,
  toLocalInputFromDate,
  toLocalInputValue,
};
