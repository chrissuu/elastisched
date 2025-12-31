const state = {
  blobs: [],
  view: "day",
  anchorDate: new Date(),
  editingBlobId: null,
  selectionMode: false,
  selectionStep: null,
  pendingDefaultRange: null,
  pendingSchedulableRange: null,
};

const dateLabel = document.getElementById("dateLabel");
const tabs = document.querySelectorAll(".tab");
const views = {
  day: document.getElementById("viewDay"),
  week: document.getElementById("viewWeek"),
  month: document.getElementById("viewMonth"),
  year: document.getElementById("viewYear"),
};
const formPanel = document.getElementById("formPanel");
const toggleFormBtn = document.getElementById("toggleFormBtn");
const closeFormBtn = document.getElementById("closeFormBtn");
const formStatus = document.getElementById("formStatus");
const blobForm = document.getElementById("blobForm");
const formTitle = document.getElementById("formTitle");
const formSubmitBtn = document.getElementById("formSubmitBtn");
const todayBtn = document.getElementById("todayBtn");
const prevDayBtn = document.getElementById("prevDayBtn");
const nextDayBtn = document.getElementById("nextDayBtn");
const goTodayBtn = document.getElementById("goTodayBtn");
const brandTitle = document.getElementById("brandTitle");
const brandSubtitle = document.getElementById("brandSubtitle");
const minuteGranularity = Math.max(1, Number(window.APP_CONFIG?.minuteGranularity || 5));

if (window.APP_CONFIG) {
  brandTitle.textContent = window.APP_CONFIG.scheduleName || brandTitle.textContent;
  brandSubtitle.textContent = window.APP_CONFIG.subtitle || brandSubtitle.textContent;
}

const demoBlobs = [
  {
    id: "demo-1",
    name: "Daily Review",
    default_scheduled_timerange: {
      start: "2024-05-21T08:00:00Z",
      end: "2024-05-21T08:20:00Z",
    },
    schedulable_timerange: {
      start: "2024-05-21T07:30:00Z",
      end: "2024-05-21T09:00:00Z",
    },
    tags: ["admin"],
  },
  {
    id: "demo-2",
    name: "Roadmap Deep Dive",
    default_scheduled_timerange: {
      start: "2024-05-21T10:00:00Z",
      end: "2024-05-21T11:15:00Z",
    },
    schedulable_timerange: {
      start: "2024-05-21T09:00:00Z",
      end: "2024-05-21T12:00:00Z",
    },
    tags: ["deep"],
  },
  {
    id: "demo-3",
    name: "Prototype Build",
    default_scheduled_timerange: {
      start: "2024-05-21T13:00:00Z",
      end: "2024-05-21T14:20:00Z",
    },
    schedulable_timerange: {
      start: "2024-05-21T12:00:00Z",
      end: "2024-05-21T15:00:00Z",
    },
    tags: ["focus"],
  },
];

function toDate(value) {
  return value ? new Date(value) : null;
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

function setDateLabel(text) {
  dateLabel.textContent = text;
}

function formatDayLabel(date) {
  return date.toLocaleDateString(undefined, {
    weekday: "long",
    month: "long",
    day: "numeric",
    year: "numeric",
  });
}

function formatWeekLabel(date) {
  return `Week of ${date.toLocaleDateString(undefined, {
    month: "long",
    day: "numeric",
    year: "numeric",
  })}`;
}

function formatMonthLabel(date) {
  return date.toLocaleDateString(undefined, { month: "long", year: "numeric" });
}

function renderDay() {
  const dayStart = startOfDay(state.anchorDate);
  const dayEnd = addDays(dayStart, 1);
  const hourHeight = 44;
  const hours = Array.from({ length: 24 }, (_, idx) => {
    const hour = idx % 24;
    const labelHour = hour === 0 ? 12 : hour > 12 ? hour - 12 : hour;
    const suffix = hour < 12 ? "AM" : "PM";
    return `${labelHour} ${suffix}`;
  });

  const blocks = state.blobs
    .map((blob) => {
      const start = toDate(
        blob.realized_timerange?.start || blob.default_scheduled_timerange?.start
      );
      const end = toDate(
        blob.realized_timerange?.end || blob.default_scheduled_timerange?.end
      );
      const schedStart = toDate(blob.schedulable_timerange?.start);
      const schedEnd = toDate(blob.schedulable_timerange?.end);
      if (!start || !end) return null;
      if (!overlaps(dayStart, dayEnd, start, end)) return null;
      const clampedStart = start < dayStart ? dayStart : start;
      const clampedEnd = end > dayEnd ? dayEnd : end;
      const minutes = (clampedEnd - clampedStart) / 60000;
      if (minutes <= 0) return null;
      const startMin = (clampedStart - dayStart) / 60000;
      const endMin = (clampedEnd - dayStart) / 60000;
      return {
        id: blob.id,
        title: blob.name,
        time: formatTimeRange(
          blob.realized_timerange?.start || blob.default_scheduled_timerange.start,
          blob.realized_timerange?.end || blob.default_scheduled_timerange.end
        ),
        type: getTagType(blob.tags),
        top: (startMin / 60) * hourHeight,
        height: Math.max(18, (minutes / 60) * hourHeight),
        startMin,
        endMin,
        schedStart,
        schedEnd,
      };
    })
    .filter(Boolean)
    .sort((a, b) => a.top - b.top);

  layoutBlocks(blocks);

  const hoursHtml = hours.map((hour) => `<div class="hour">${hour}</div>`).join("");
  const blockHtml = blocks
    .map(
      (block) => `
        <div class="day-block ${block.type}" style="top: ${block.top}px; height: ${block.height}px; --column: ${block.column}; --columns: ${block.columns};" data-blob-id="${block.id}" data-sched-start="${block.schedStart?.toISOString() || ""}" data-sched-end="${block.schedEnd?.toISOString() || ""}">
          <span>${block.title}</span>
          <span class="event-time">${block.time}</span>
        </div>
      `
    )
    .join("");

  views.day.innerHTML = `
    <div class="day-grid" style="--hour-height: ${hourHeight}px;">
      <div class="hours">${hoursHtml}</div>
      <div class="day-track">
        <div class="schedulable-overlay" id="schedulableOverlay"></div>
        <div class="selection-overlay default-range" id="selectionOverlayDefault"></div>
        <div class="selection-overlay schedulable-range" id="selectionOverlaySchedulable"></div>
        ${blockHtml || "<div class='day-empty'>No events yet</div>"}
      </div>
    </div>
  `;

  const overlay = views.day.querySelector("#schedulableOverlay");
  const dayTrack = views.day.querySelector(".day-track");
  const selectionOverlayDefault = views.day.querySelector("#selectionOverlayDefault");
  const selectionOverlaySchedulable = views.day.querySelector("#selectionOverlaySchedulable");
  const blocksEls = views.day.querySelectorAll(".day-block");
  blocksEls.forEach((blockEl) => {
    blockEl.addEventListener("mouseenter", () => {
      blockEl.classList.add("hovered");
      const schedStart = toDate(blockEl.getAttribute("data-sched-start"));
      const schedEnd = toDate(blockEl.getAttribute("data-sched-end"));
      if (!schedStart || !schedEnd) return;
      const overlayStart = schedStart < dayStart ? dayStart : schedStart;
      const overlayEnd = schedEnd > dayEnd ? dayEnd : schedEnd;
      const minutes = (overlayEnd - overlayStart) / 60000;
      const top = (overlayStart - dayStart) / 60000;
      overlay.style.top = `${(top / 60) * hourHeight}px`;
      overlay.style.height = `${Math.max(18, (minutes / 60) * hourHeight)}px`;
      overlay.classList.toggle("overflow-top", schedStart < dayStart);
      overlay.classList.toggle("overflow-bottom", schedEnd > dayEnd);
      overlay.classList.add("active");
    });
    blockEl.addEventListener("mouseleave", () => {
      blockEl.classList.remove("hovered");
      overlay.classList.remove("active", "overflow-top", "overflow-bottom");
    });
  });

  if (state.selectionMode) {
    let clickStart = null;
    const padding = 8;
    const trackMinutes = 24 * 60;
    const trackHeight = hourHeight * 24;

    const toMinutes = (clientY) => {
      const rect = dayTrack.getBoundingClientRect();
      const y = Math.min(
        Math.max(clientY - rect.top - padding, 0),
        trackHeight
      );
      return clampToGranularity(Math.round((y / trackHeight) * trackMinutes));
    };

    const updateSelectionOverlay = (overlayEl, startMin, endMin) => {
      const top = (startMin / 60) * hourHeight;
      const height = Math.max(12, ((endMin - startMin) / 60) * hourHeight);
      overlayEl.style.top = `${top}px`;
      overlayEl.style.height = `${height}px`;
      overlayEl.classList.add("active");
    };

    const finalizeRange = (startMin, endMin) => {
      const startDate = new Date(dayStart.getTime() + startMin * 60000);
      const endDate = new Date(dayStart.getTime() + endMin * 60000);
      if (state.selectionStep === "default") {
        state.pendingDefaultRange = { start: startDate, end: endDate };
        state.selectionStep = "schedulable";
        formStatus.textContent = "Click start/end for schedulable range.";
      } else if (state.selectionStep === "schedulable") {
        state.pendingSchedulableRange = { start: startDate, end: endDate };
        state.selectionMode = false;
        state.selectionStep = null;
        selectionOverlayDefault.classList.add("active");
        selectionOverlaySchedulable.classList.add("active");
        const defaultRange = state.pendingDefaultRange;
        const schedRange = state.pendingSchedulableRange;
        if (defaultRange && schedRange) {
          blobForm.defaultStart.value = toLocalInputFromDate(defaultRange.start);
          blobForm.defaultEnd.value = toLocalInputFromDate(defaultRange.end);
          blobForm.schedulableStart.value = toLocalInputFromDate(schedRange.start);
          blobForm.schedulableEnd.value = toLocalInputFromDate(schedRange.end);
        }
        formStatus.textContent = "Ranges captured. Fill details and create.";
      }
    };

    const onClick = (event) => {
      if (event.button !== 0) return;
      if (event.target.closest(".day-block")) return;
      if (state.selectionStep === null) return;
      const minutes = toMinutes(event.clientY);
      if (clickStart === null) {
        clickStart = minutes;
        const overlayEl =
          state.selectionStep === "default"
            ? selectionOverlayDefault
            : selectionOverlaySchedulable;
        const endMin = Math.min(trackMinutes, minutes + minuteGranularity);
        updateSelectionOverlay(overlayEl, minutes, endMin);
      } else {
        const startMin = Math.min(clickStart, minutes);
        const endMin = Math.min(
          trackMinutes,
          Math.max(clickStart + minuteGranularity, minutes)
        );
        const overlayEl =
          state.selectionStep === "default"
            ? selectionOverlayDefault
            : selectionOverlaySchedulable;
        updateSelectionOverlay(overlayEl, startMin, endMin);
        finalizeRange(startMin, endMin);
        clickStart = null;
      }
    };

    const onMouseMove = (event) => {
      if (clickStart === null) return;
      if (state.selectionStep === null) return;
      const minutes = toMinutes(event.clientY);
      const startMin = Math.min(clickStart, minutes);
      const endMin = Math.min(
        trackMinutes,
        Math.max(clickStart + minuteGranularity, minutes)
      );
      const overlayEl =
        state.selectionStep === "default"
          ? selectionOverlayDefault
          : selectionOverlaySchedulable;
      updateSelectionOverlay(overlayEl, startMin, endMin);
    };

    dayTrack.addEventListener("click", onClick);
    dayTrack.addEventListener("mousemove", onMouseMove);
  }

  setDateLabel(formatDayLabel(state.anchorDate));
}

function renderWeek() {
  const dayOfWeek = state.anchorDate.getDay();
  const monday = addDays(state.anchorDate, dayOfWeek === 0 ? -6 : 1 - dayOfWeek);
  const days = Array.from({ length: 7 }, (_, idx) => addDays(monday, idx));
  const hourHeight = 44;
  const hours = Array.from({ length: 24 }, (_, idx) => {
    const hour = idx % 24;
    const labelHour = hour === 0 ? 12 : hour > 12 ? hour - 12 : hour;
    const suffix = hour < 12 ? "AM" : "PM";
    return `${labelHour} ${suffix}`;
  });

  const columns = days
    .map((date) => {
      const dayStart = startOfDay(date);
      const dayEnd = addDays(dayStart, 1);
      const blocks = state.blobs
        .map((blob) => {
          const start = toDate(
            blob.realized_timerange?.start || blob.default_scheduled_timerange?.start
          );
          const end = toDate(
            blob.realized_timerange?.end || blob.default_scheduled_timerange?.end
          );
          const schedStart = toDate(blob.schedulable_timerange?.start);
          const schedEnd = toDate(blob.schedulable_timerange?.end);
          if (!start || !end) return null;
          if (!overlaps(dayStart, dayEnd, start, end)) return null;
          const clampedStart = start < dayStart ? dayStart : start;
          const clampedEnd = end > dayEnd ? dayEnd : end;
          const minutes = (clampedEnd - clampedStart) / 60000;
          if (minutes <= 0) return null;
          const startMin = (clampedStart - dayStart) / 60000;
          const endMin = (clampedEnd - dayStart) / 60000;
          return {
            id: blob.id,
            title: blob.name,
            time: formatTimeRange(
              blob.realized_timerange?.start || blob.default_scheduled_timerange.start,
              blob.realized_timerange?.end || blob.default_scheduled_timerange.end
            ),
            type: getTagType(blob.tags),
            top: (startMin / 60) * hourHeight,
            height: Math.max(18, (minutes / 60) * hourHeight),
            startMin,
            endMin,
            schedStart,
            schedEnd,
          };
        })
        .filter(Boolean)
        .sort((a, b) => a.top - b.top);

      layoutBlocks(blocks);

      const blockHtml = blocks
        .map(
          (block) => `
            <div class="day-block ${block.type}" style="top: ${block.top}px; height: ${block.height}px; --column: ${block.column}; --columns: ${block.columns};" data-blob-id="${block.id}" data-sched-start="${block.schedStart?.toISOString() || ""}" data-sched-end="${block.schedEnd?.toISOString() || ""}">
              <span>${block.title}</span>
              <span class="event-time">${block.time}</span>
            </div>
          `
        )
        .join("");
      return `
        <div class="week-day-column" style="--hour-height: ${hourHeight}px;">
          <div class="week-day-label">
            <button data-date="${date.toISOString()}">
              ${date.toLocaleDateString(undefined, {
                weekday: "short",
                month: "short",
                day: "numeric",
              })}
            </button>
          </div>
          <div class="week-day-track">
            <div class="schedulable-overlay"></div>
            ${blockHtml || "<div class='day-empty'>No events yet</div>"}
          </div>
        </div>
      `;
    })
    .join("");

  const hoursHtml = hours.map((hour) => `<div class="hour">${hour}</div>`).join("");
  views.week.innerHTML = `
    <div class="week-timeline" style="--hour-height: ${hourHeight}px;">
      <div class="week-hours">${hoursHtml}</div>
      <div class="week-days">${columns}</div>
    </div>
  `;

  const weekStart = startOfDay(monday);
  const weekEnd = addDays(weekStart, 7);
  const dayColumns = Array.from(views.week.querySelectorAll(".week-day-column"));
  const dayTracks = dayColumns.map((column, index) => {
    const dayStart = startOfDay(days[index]);
    const dayEnd = addDays(dayStart, 1);
    return {
      track: column.querySelector(".week-day-track"),
      overlay: column.querySelector(".schedulable-overlay"),
      dayStart,
      dayEnd,
    };
  });

  dayColumns.forEach((column) => {
    column.querySelectorAll(".day-block").forEach((blockEl) => {
      blockEl.addEventListener("mouseenter", () => {
        blockEl.classList.add("hovered");
        const schedStart = toDate(blockEl.getAttribute("data-sched-start"));
        const schedEnd = toDate(blockEl.getAttribute("data-sched-end"));
        if (!schedStart || !schedEnd) return;
        dayTracks.forEach(({ overlay, dayStart, dayEnd }) => {
          const overlapStart = schedStart < dayStart ? dayStart : schedStart;
          const overlapEnd = schedEnd > dayEnd ? dayEnd : schedEnd;
          if (overlapEnd <= overlapStart) {
            overlay.classList.remove("active", "overflow-top", "overflow-bottom");
            return;
          }
          const minutes = (overlapEnd - overlapStart) / 60000;
          const top = (overlapStart - dayStart) / 60000;
          overlay.style.top = `${(top / 60) * hourHeight}px`;
          overlay.style.height = `${Math.max(18, (minutes / 60) * hourHeight)}px`;
          overlay.classList.toggle("overflow-top", schedStart < weekStart && dayStart.getTime() === weekStart.getTime());
          overlay.classList.toggle("overflow-bottom", schedEnd > weekEnd && dayEnd.getTime() === weekEnd.getTime());
          overlay.classList.add("active");
        });
      });
      blockEl.addEventListener("mouseleave", () => {
        blockEl.classList.remove("hovered");
        dayTracks.forEach(({ overlay }) => {
          overlay.classList.remove("active", "overflow-top", "overflow-bottom");
        });
      });
    });
  });
  setDateLabel(formatWeekLabel(monday));
}

function renderMonth() {
  const monthStart = new Date(state.anchorDate.getFullYear(), state.anchorDate.getMonth(), 1);
  const monthEnd = new Date(state.anchorDate.getFullYear(), state.anchorDate.getMonth() + 1, 1);
  const weeks = [];
  let cursor = monthStart;
  while (cursor < monthEnd) {
    weeks.push(cursor);
    cursor = addDays(cursor, 7);
  }

  const cards = weeks
    .map((weekStart, idx) => {
      const weekEnd = addDays(weekStart, 7);
      const chips = Array.from({ length: 7 }, (_, offset) => addDays(weekStart, offset))
        .filter((date) => date.getMonth() === monthStart.getMonth())
        .map(
          (date) => `
          <button class="day-chip" data-date="${date.toISOString()}">
            ${date.getDate()}
          </button>
        `
        )
        .join("");
      const events = state.blobs.filter((blob) => {
        const start = toDate(blob.default_scheduled_timerange?.start);
        const end = toDate(blob.default_scheduled_timerange?.end);
        return start && end && overlaps(weekStart, weekEnd, start, end);
      });
      return `
        <div class="card">
          <div class="card-title">Week ${idx + 1}</div>
          <div class="day-chips">${chips || "<span class='card-summary'>No days</span>"}</div>
          <div class="card-summary">${events.length} sessions</div>
          <div class="card-event"><span>Focus blocks</span><span>${Math.max(
            0,
            events.length - 2
          )}</span></div>
          <div class="card-event"><span>Compression</span><span>${events.length > 6 ? "High" : "Low"}</span></div>
        </div>
      `;
    })
    .join("");

  views.month.innerHTML = `<div class="month-grid">${cards}</div>`;
  setDateLabel(formatMonthLabel(state.anchorDate));
}

function renderYear() {
  const year = state.anchorDate.getFullYear();
  const quarters = [0, 3, 6, 9].map((month) => new Date(year, month, 1));
  const cards = quarters
    .map((quarterStart, idx) => {
      const quarterEnd = new Date(year, quarterStart.getMonth() + 3, 1);
      const events = state.blobs.filter((blob) => {
        const start = toDate(blob.default_scheduled_timerange?.start);
        const end = toDate(blob.default_scheduled_timerange?.end);
        return start && end && overlaps(quarterStart, quarterEnd, start, end);
      });
      return `
        <div class="card">
          <div class="card-title">Q${idx + 1}</div>
          <div class="card-summary">${events.length} sessions</div>
          <div class="card-event"><span>Peak month</span><span>${quarterStart.toLocaleDateString(undefined, {
            month: "short",
          })}</span></div>
          <div class="card-event"><span>Flow score</span><span>${(0.6 + events.length / 100).toFixed(2)}</span></div>
        </div>
      `;
    })
    .join("");

  views.year.innerHTML = `<div class="year-grid">${cards}</div>`;
  setDateLabel(`Year ${year}`);
}

function renderAll() {
  renderDay();
  renderWeek();
  renderMonth();
  renderYear();
}

function setActive(view) {
  state.view = view;
  tabs.forEach((tab) => tab.classList.toggle("active", tab.dataset.view === view));
  Object.entries(views).forEach(([key, el]) => {
    el.classList.toggle("active", key === view);
  });

  if (view === "day") {
    renderDay();
  } else if (view === "week") {
    renderWeek();
  } else if (view === "month") {
    renderMonth();
  } else if (view === "year") {
    renderYear();
  }
}

const API_BASE = window.location.origin;

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

function toggleForm(show) {
  const isActive = typeof show === "boolean" ? show : !formPanel.classList.contains("active");
  formPanel.classList.toggle("active", isActive);
}

function startInteractiveCreate() {
  state.selectionMode = true;
  state.selectionStep = "default";
  state.pendingDefaultRange = null;
  state.pendingSchedulableRange = null;
  formStatus.textContent = "Click start/end for default range.";
  renderDay();
}

function setFormMode(mode) {
  if (mode === "edit") {
    formTitle.textContent = "Edit blob";
    formSubmitBtn.textContent = "Update";
  } else {
    formTitle.textContent = "Create a blob";
    formSubmitBtn.textContent = "Create";
  }
}

function openEditForm(blob) {
  state.selectionMode = false;
  state.selectionStep = null;
  state.pendingDefaultRange = null;
  state.pendingSchedulableRange = null;
  blobForm.name.value = blob.name || "";
  blobForm.description.value = blob.description || "";
  blobForm.defaultStart.value = toLocalInputValue(blob.default_scheduled_timerange?.start);
  blobForm.defaultEnd.value = toLocalInputValue(blob.default_scheduled_timerange?.end);
  blobForm.schedulableStart.value = toLocalInputValue(blob.schedulable_timerange?.start);
  blobForm.schedulableEnd.value = toLocalInputValue(blob.schedulable_timerange?.end);
  state.editingBlobId = blob.id;
  setFormMode("edit");
  formStatus.textContent = "";
  toggleForm(true);
}

function resetFormMode() {
  state.editingBlobId = null;
  state.selectionMode = false;
  state.selectionStep = null;
  state.pendingDefaultRange = null;
  state.pendingSchedulableRange = null;
  blobForm.reset();
  setFormMode("create");
  formStatus.textContent = "";
  const defaultOverlay = document.getElementById("selectionOverlayDefault");
  const schedOverlay = document.getElementById("selectionOverlaySchedulable");
  if (defaultOverlay) {
    defaultOverlay.classList.remove("active");
    defaultOverlay.style.top = "";
    defaultOverlay.style.height = "";
  }
  if (schedOverlay) {
    schedOverlay.classList.remove("active");
    schedOverlay.style.top = "";
    schedOverlay.style.height = "";
  }
}

function goToDate(dateIso) {
  const parsed = new Date(dateIso);
  if (!Number.isNaN(parsed.getTime())) {
    state.anchorDate = parsed;
    renderAll();
    setActive("day");
  }
}

blobForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  formStatus.textContent = "Saving...";
  const formData = new FormData(blobForm);
  const payload = {
    name: formData.get("name"),
    description: formData.get("description") || null,
    tz: "UTC",
    default_scheduled_timerange: {
      start: toIso(formData.get("defaultStart")),
      end: toIso(formData.get("defaultEnd")),
    },
    schedulable_timerange: {
      start: toIso(formData.get("schedulableStart")),
      end: toIso(formData.get("schedulableEnd")),
    },
    policy: {},
    dependencies: [],
    tags: [],
  };

  try {
    const isEditing = Boolean(state.editingBlobId);
    const endpoint = isEditing ? `${API_BASE}/blobs/${state.editingBlobId}` : `${API_BASE}/blobs`;
    const response = await fetch(endpoint, {
      method: isEditing ? "PUT" : "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });
    if (!response.ok) {
      let detail = "Failed to create blob";
      const contentType = response.headers.get("content-type") || "";
      if (contentType.includes("application/json")) {
        const data = await response.json();
        detail = data.detail || detail;
      } else {
        detail = (await response.text()) || detail;
      }
      throw new Error(detail);
    }
    blobForm.reset();
    formStatus.textContent = isEditing ? "Updated." : "Created.";
    toggleForm(false);
    resetFormMode();
    await fetchBlobs();
  } catch (error) {
    formStatus.textContent = error?.message || "Error saving blob.";
  }
});

tabs.forEach((tab) => {
  tab.addEventListener("click", () => setActive(tab.dataset.view));
});

document.addEventListener("click", (event) => {
  const target = event.target.closest("[data-date]");
  if (!target) return;
  const dateIso = target.getAttribute("data-date");
  if (dateIso) {
    goToDate(dateIso);
  }
});

document.addEventListener("click", (event) => {
  const target = event.target.closest("[data-blob-id]");
  if (!target) return;
  const blobId = target.getAttribute("data-blob-id");
  const blob = state.blobs.find((item) => item.id === blobId);
  if (blob) {
    openEditForm(blob);
  }
});

toggleFormBtn.addEventListener("click", () => {
  resetFormMode();
  toggleForm(true);
  setActive("day");
  startInteractiveCreate();
});
closeFormBtn.addEventListener("click", () => {
  toggleForm(false);
  resetFormMode();
});
todayBtn.addEventListener("click", () => {
  state.anchorDate = new Date();
  renderAll();
  setActive("day");
});

function moveDay(offset) {
  state.anchorDate = addDays(state.anchorDate, offset);
  renderAll();
  setActive("day");
}

prevDayBtn.addEventListener("click", () => moveDay(-1));
nextDayBtn.addEventListener("click", () => moveDay(1));
goTodayBtn.addEventListener("click", () => {
  state.anchorDate = new Date();
  renderAll();
  setActive("day");
});

resetFormMode();
setActive("day");
fetchBlobs();
