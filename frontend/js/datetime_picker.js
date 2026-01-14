import { minuteGranularity } from "./core.js";

let activeField = null;
let activePopover = null;
let activeState = null;

function pad(value) {
  return `${value}`.padStart(2, "0");
}

function parseLocalValue(value) {
  if (!value) return null;
  const [datePart, timePart] = value.split("T");
  if (!datePart || !timePart) return null;
  const [year, month, day] = datePart.split("-").map(Number);
  const [hour, minute] = timePart.split(":").map(Number);
  if ([year, month, day, hour, minute].some((item) => Number.isNaN(item))) {
    return null;
  }
  return { year, month, day, hour, minute };
}

function parseDateValue(value) {
  if (!value) return null;
  const [year, month, day] = value.split("-").map(Number);
  if ([year, month, day].some((item) => Number.isNaN(item))) {
    return null;
  }
  return { year, month, day };
}

function formatLocalValue({ year, month, day, hour, minute }) {
  return `${year}-${pad(month)}-${pad(day)}T${pad(hour)}:${pad(minute)}`;
}

function formatDisplay(value, mode) {
  if (mode === "date") {
    const parsed = parseDateValue(value);
    if (!parsed) return "";
    const date = new Date(parsed.year, parsed.month - 1, parsed.day);
    if (Number.isNaN(date.getTime())) return "";
    return date.toLocaleDateString(undefined, {
      month: "short",
      day: "numeric",
      year: "numeric",
    });
  }
  const parsed = parseLocalValue(value);
  if (!parsed) return "";
  const date = new Date(
    parsed.year,
    parsed.month - 1,
    parsed.day,
    parsed.hour,
    parsed.minute
  );
  if (Number.isNaN(date.getTime())) return "";
  return date.toLocaleString(undefined, {
    month: "short",
    day: "numeric",
    year: "numeric",
    hour: "2-digit",
    minute: "2-digit",
  });
}

function getMonthLabel(year, monthIndex) {
  const date = new Date(year, monthIndex, 1);
  return date.toLocaleDateString(undefined, { month: "long", year: "numeric" });
}

function getDaysForMonth(year, monthIndex) {
  const start = new Date(year, monthIndex, 1);
  const startDay = start.getDay();
  const daysInMonth = new Date(year, monthIndex + 1, 0).getDate();
  const prevMonthDays = new Date(year, monthIndex, 0).getDate();
  const days = [];
  for (let i = startDay - 1; i >= 0; i -= 1) {
    days.push({ day: prevMonthDays - i, isOther: true });
  }
  for (let i = 1; i <= daysInMonth; i += 1) {
    days.push({ day: i, isOther: false });
  }
  while (days.length % 7 !== 0) {
    days.push({ day: days.length % 7 + 1, isOther: true });
  }
  return days;
}

function buildPopover(mode) {
  const popover = document.createElement("div");
  popover.className = "datetime-popover";
  popover.innerHTML = `
    <div class="datetime-header">
      <button type="button" class="ghost small" data-action="prev-month">‹</button>
      <div class="datetime-title"></div>
      <button type="button" class="ghost small" data-action="next-month">›</button>
    </div>
    <div class="datetime-weekdays">
      ${["S", "M", "T", "W", "T", "F", "S"].map((day) => `<span>${day}</span>`).join("")}
    </div>
    <div class="datetime-grid"></div>
    ${
      mode === "date"
        ? ""
        : `
    <div class="datetime-time">
      <label>
        Hour
        <select data-action="hour"></select>
      </label>
      <label>
        Minute
        <select data-action="minute"></select>
      </label>
    </div>
    `
    }
    <div class="datetime-actions">
      <button type="button" class="ghost small" data-action="clear">Clear</button>
      <button type="button" class="ghost small" data-action="today">Today</button>
      <button type="button" class="primary" data-action="apply">Apply</button>
    </div>
  `;
  return popover;
}

function closePopover() {
  if (activePopover) {
    activePopover.remove();
  }
  activePopover = null;
  activeField = null;
  activeState = null;
}

function updateDisplayForField(field) {
  const display = field.querySelector(".datetime-display");
  const hidden = field.querySelector("[data-datetime-input]");
  if (!display || !hidden) return;
  const mode = field.dataset.mode || "datetime";
  display.value = formatDisplay(hidden.value, mode);
}

function renderPopover() {
  if (!activePopover || !activeState) return;
  const title = activePopover.querySelector(".datetime-title");
  const grid = activePopover.querySelector(".datetime-grid");
  const hourSelect = activePopover.querySelector('[data-action="hour"]');
  const minuteSelect = activePopover.querySelector('[data-action="minute"]');

  title.textContent = getMonthLabel(activeState.year, activeState.monthIndex);
  grid.innerHTML = "";
  const days = getDaysForMonth(activeState.year, activeState.monthIndex);
  days.forEach((item) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = `datetime-day ${item.isOther ? "other" : ""}`;
    button.textContent = item.day;
    if (!item.isOther && item.day === activeState.day) {
      button.classList.add("active");
    }
    button.addEventListener("click", () => {
      if (item.isOther) return;
      activeState.day = item.day;
      renderPopover();
    });
    grid.appendChild(button);
  });

  if (activeState.mode !== "date" && hourSelect && minuteSelect) {
    hourSelect.innerHTML = "";
    for (let hour = 0; hour < 24; hour += 1) {
      const option = document.createElement("option");
      option.value = `${hour}`;
      option.textContent = pad(hour);
      if (hour === activeState.hour) {
        option.selected = true;
      }
      hourSelect.appendChild(option);
    }

    minuteSelect.innerHTML = "";
    const step = Math.max(1, minuteGranularity);
    for (let minute = 0; minute < 60; minute += step) {
      const option = document.createElement("option");
      option.value = `${minute}`;
      option.textContent = pad(minute);
      if (minute === activeState.minute) {
        option.selected = true;
      }
      minuteSelect.appendChild(option);
    }
  }
}

function setPopoverPosition(field, popover) {
  const rect = field.getBoundingClientRect();
  const padding = 12;
  let left = rect.left;
  let top = rect.bottom + 8;
  if (left + popover.offsetWidth > window.innerWidth - padding) {
    left = window.innerWidth - popover.offsetWidth - padding;
  }
  left = Math.max(padding, left);
  if (top + popover.offsetHeight > window.innerHeight - padding) {
    top = rect.top - popover.offsetHeight - 8;
  }
  top = Math.max(padding, top);
  popover.style.left = `${left}px`;
  popover.style.top = `${top}px`;
}

function openPopover(field) {
  closePopover();
  const hidden = field.querySelector("[data-datetime-input]");
  if (!hidden) return;
  const mode = field.dataset.mode || "datetime";
  const parsed = mode === "date" ? parseDateValue(hidden.value) : parseLocalValue(hidden.value);
  const now = new Date();
  const initial = parsed || {
    year: now.getFullYear(),
    month: now.getMonth() + 1,
    day: now.getDate(),
    hour: now.getHours(),
    minute: now.getMinutes() - (now.getMinutes() % Math.max(1, minuteGranularity)),
  };
  activeState = {
    year: initial.year,
    monthIndex: initial.month - 1,
    day: initial.day,
    hour: mode === "date" ? 0 : initial.hour,
    minute: mode === "date" ? 0 : initial.minute,
    mode,
  };
  activeField = field;
  activePopover = buildPopover(mode);
  document.body.appendChild(activePopover);
  activePopover.addEventListener("click", (event) => {
    event.stopPropagation();
  });
  activePopover.addEventListener("click", handlePopoverClick);
  const updateTime = (event) => {
    if (!activePopover || !activeState) return;
    const action = event.target.closest("[data-action]")?.dataset?.action;
    if (action === "hour") {
      activeState.hour = Number(event.target.value);
    } else if (action === "minute") {
      activeState.minute = Number(event.target.value);
    }
  };
  if (mode !== "date") {
    activePopover.addEventListener("input", updateTime);
    activePopover.addEventListener("change", updateTime);
  }
  renderPopover();
  setPopoverPosition(field, activePopover);
}

function applySelection() {
  if (!activeField || !activeState) return;
  const hidden = activeField.querySelector("[data-datetime-input]");
  if (!hidden) return;
  if (activeState.mode === "date") {
    hidden.value = `${activeState.year}-${pad(activeState.monthIndex + 1)}-${pad(
      activeState.day
    )}`;
  } else {
    hidden.value = formatLocalValue({
      year: activeState.year,
      month: activeState.monthIndex + 1,
      day: activeState.day,
      hour: activeState.hour,
      minute: activeState.minute,
    });
  }
  hidden.dispatchEvent(new Event("change", { bubbles: true }));
  updateDisplayForField(activeField);
  closePopover();
}

function clearSelection() {
  if (!activeField) return;
  const hidden = activeField.querySelector("[data-datetime-input]");
  if (!hidden) return;
  hidden.value = "";
  hidden.dispatchEvent(new Event("change", { bubbles: true }));
  updateDisplayForField(activeField);
  closePopover();
}

function handlePopoverClick(event) {
  if (!activePopover || !activeState) return;
  const action = event.target.closest("[data-action]")?.dataset?.action;
  if (!action) return;
  event.preventDefault();
  event.stopPropagation();
  if (action === "prev-month") {
    activeState.monthIndex -= 1;
    if (activeState.monthIndex < 0) {
      activeState.monthIndex = 11;
      activeState.year -= 1;
    }
    const maxDay = new Date(activeState.year, activeState.monthIndex + 1, 0).getDate();
    if (activeState.day > maxDay) {
      activeState.day = maxDay;
    }
    renderPopover();
    return;
  }
  if (action === "next-month") {
    activeState.monthIndex += 1;
    if (activeState.monthIndex > 11) {
      activeState.monthIndex = 0;
      activeState.year += 1;
    }
    const maxDay = new Date(activeState.year, activeState.monthIndex + 1, 0).getDate();
    if (activeState.day > maxDay) {
      activeState.day = maxDay;
    }
    renderPopover();
    return;
  }
  if (action === "today") {
    const now = new Date();
    activeState.year = now.getFullYear();
    activeState.monthIndex = now.getMonth();
    activeState.day = now.getDate();
    activeState.hour = now.getHours();
    activeState.minute = now.getMinutes() - (now.getMinutes() % Math.max(1, minuteGranularity));
    renderPopover();
    return;
  }
  if (action === "clear") {
    clearSelection();
    return;
  }
  if (action === "apply") {
    applySelection();
  }
}

function bindPopoverEvents() {
  document.addEventListener("click", (event) => {
    if (!activePopover) return;
    if (event.target.closest(".datetime-popover")) return;
    if (event.target.closest(".datetime-field")) return;
    closePopover();
  });
  window.addEventListener("keydown", (event) => {
    if (!activePopover) return;
    if (event.key === "Escape") {
      closePopover();
    }
  });
  window.addEventListener("resize", () => {
    if (activePopover && activeField) {
      setPopoverPosition(activeField, activePopover);
    }
  });
}

function bindDateTimePickers() {
  const fields = document.querySelectorAll(".datetime-field");
  fields.forEach((field) => {
    const display = field.querySelector(".datetime-display");
    const trigger = field.querySelector(".datetime-trigger");
    const hidden = field.querySelector("[data-datetime-input]");
    if (!display || !hidden) return;
    updateDisplayForField(field);
    if (field.dataset.datetimeBound) {
      return;
    }
    field.dataset.datetimeBound = "true";
    const open = (event) => {
      event.preventDefault();
      openPopover(field);
    };
    display.addEventListener("click", open);
    if (trigger) {
      trigger.addEventListener("click", open);
    }
    hidden.addEventListener("change", () => updateDisplayForField(field));
  });
  if (!document.body.dataset.datetimePickerBound) {
    document.body.dataset.datetimePickerBound = "true";
    bindPopoverEvents();
  }
}

function syncDateTimeDisplays() {
  document.querySelectorAll(".datetime-field").forEach((field) => {
    updateDisplayForField(field);
  });
}

export { bindDateTimePickers, syncDateTimeDisplays };
