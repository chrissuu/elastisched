import { API_BASE, appConfig, saveSettings, state } from "./core.js";
import { dom } from "./dom.js";
import { addDays, getLocalTimeZone, getWeekStart, toIso, toLocalInputValue } from "./utils.js";
import { startInteractiveCreate } from "./render.js";

let refreshView = null;
const recurrenceFieldGroups = document.querySelectorAll(".recurrence-fields");
const WEEK_DAYS = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];

function setRefreshHandler(handler) {
  refreshView = handler;
}

function toggleForm(show) {
  const isActive = typeof show === "boolean" ? show : !dom.formPanel.classList.contains("active");
  dom.formPanel.classList.toggle("active", isActive);
}

function toggleSettings(show) {
  const isActive = typeof show === "boolean" ? show : !dom.settingsModal.classList.contains("active");
  dom.settingsModal.classList.toggle("active", isActive);
  dom.settingsPanel.classList.toggle("active", isActive);
  dom.settingsModal.setAttribute("aria-hidden", (!isActive).toString());
}

function hydrateSettingsForm() {
  dom.settingsForm.scheduleName.value = appConfig.scheduleName || "";
  dom.settingsForm.subtitle.value = appConfig.subtitle || "";
  dom.settingsForm.minuteGranularity.value = appConfig.minuteGranularity || 5;
}

function setFormMode(mode) {
  if (mode === "edit") {
    dom.formTitle.textContent = "Edit recurrence";
    dom.formSubmitBtn.textContent = "Update";
  } else {
    dom.formTitle.textContent = "Create a recurrence";
    dom.formSubmitBtn.textContent = "Create";
  }
}

function updateRecurrenceUI() {
  const type = dom.recurrenceType?.value || "single";
  recurrenceFieldGroups.forEach((group) => {
    const matches = group.dataset.recurrence === type;
    group.classList.toggle("active", matches);
  });
  dom.formPanel.classList.toggle("weekly-mode", type === "weekly");
  const weeklyWrapper = dom.weeklySlots?.closest(".weekly-slots");
  if (weeklyWrapper) {
    weeklyWrapper.classList.toggle("per-slot", Boolean(dom.weeklyPerSlot?.checked));
  }
  document.querySelectorAll(".non-weekly-field input").forEach((field) => {
    const shouldDisable = type === "weekly";
    field.disabled = shouldDisable;
    field.required = !shouldDisable;
  });

  const defaultStart = dom.blobForm.defaultStart.value;
  const startDate = defaultStart ? new Date(defaultStart) : state.anchorDate;
  if (Number.isNaN(startDate.getTime())) {
    dom.recurrenceSummary.textContent = "Set a default start time to preview the cadence.";
    return;
  }
  if (type === "weekly") {
    const interval = Number(dom.blobForm.weeklyInterval.value || 1);
    const slotCount = dom.weeklySlots?.querySelectorAll(".weekly-slot").length || 0;
    dom.recurrenceSummary.textContent = `Repeats every ${interval} week(s) with ${slotCount} slot(s).`;
  } else if (type === "delta") {
    const value = Number(dom.blobForm.deltaValue.value || 1);
    const unit = dom.blobForm.deltaUnit.value || "days";
    dom.recurrenceSummary.textContent = `Repeats every ${value} ${unit}.`;
  } else if (type === "date") {
    const dateLabel = startDate.toLocaleDateString(undefined, {
      month: "long",
      day: "numeric",
    });
    dom.recurrenceSummary.textContent = `Repeats annually on ${dateLabel}.`;
  } else {
    dom.recurrenceSummary.textContent = "One-time event. Switch to create repeats.";
  }
}

function timeToMinutes(value) {
  if (!value) return null;
  const [hours, minutes] = value.split(":").map((part) => Number(part));
  if (Number.isNaN(hours) || Number.isNaN(minutes)) return null;
  return hours * 60 + minutes;
}

function pad2(value) {
  return String(value).padStart(2, "0");
}

function timeValueFromDate(date, fallback) {
  if (!date || Number.isNaN(date.getTime())) return fallback;
  return `${pad2(date.getHours())}:${pad2(date.getMinutes())}`;
}

function timeToDate(baseDate, timeValue) {
  const minutes = timeToMinutes(timeValue);
  if (minutes === null) return null;
  const result = new Date(baseDate);
  result.setHours(0, 0, 0, 0);
  result.setMinutes(minutes);
  return result;
}

function dayOffsetFromMonday(dayIndex) {
  return (dayIndex + 6) % 7;
}

function clearWeeklySlots() {
  if (dom.weeklySlots) {
    dom.weeklySlots.innerHTML = "";
  }
}

function createWeeklySlot(slotData = {}) {
  if (!dom.weeklySlots) return;
  const slot = document.createElement("div");
  slot.className = "weekly-slot";
  const slotId = typeof crypto !== "undefined" && crypto.randomUUID
    ? crypto.randomUUID()
    : `slot-${Date.now()}-${Math.random().toString(16).slice(2)}`;
  slot.dataset.slotId = slotId;
  const dayValue = slotData.day ?? 1;
  const defaultStart = slotData.defaultStart || "09:00";
  const defaultEnd = slotData.defaultEnd || "10:00";
  const schedStart = slotData.schedStart || "08:30";
  const schedEnd = slotData.schedEnd || "10:30";
  const nameValue = slotData.name || "";
  const descriptionValue = slotData.description || "";
  slot.innerHTML = `
    <div class="weekly-slot-row">
      <label>
        Day
        <select name="slotDay">
          ${WEEK_DAYS.map(
            (day, index) =>
              `<option value="${index}" ${index === dayValue ? "selected" : ""}>${day}</option>`
          ).join("")}
        </select>
      </label>
      <label>
        Default start
        <input type="time" name="slotDefaultStart" value="${defaultStart}" required />
      </label>
      <label>
        Default end
        <input type="time" name="slotDefaultEnd" value="${defaultEnd}" required />
      </label>
      <label>
        Schedulable start
        <input type="time" name="slotSchedStart" value="${schedStart}" required />
      </label>
      <label>
        Schedulable end
        <input type="time" name="slotSchedEnd" value="${schedEnd}" required />
      </label>
    </div>
    <div class="weekly-slot-row slot-meta">
      <label>
        Slot name
        <input type="text" name="slotName" value="${nameValue}" />
      </label>
      <label>
        Slot description
        <input type="text" name="slotDescription" value="${descriptionValue}" />
      </label>
    </div>
    <div class="weekly-slot-actions">
      <button type="button" class="ghost small" data-action="remove-slot">Remove</button>
    </div>
  `;
  slot.querySelector('[data-action="remove-slot"]').addEventListener("click", () => {
    slot.remove();
    updateRecurrenceUI();
    validateWeeklySlots();
  });
  slot.querySelectorAll("input, select").forEach((field) => {
    field.addEventListener("change", () => {
      updateRecurrenceUI();
      validateWeeklySlots();
    });
  });
  dom.weeklySlots.appendChild(slot);
  updateRecurrenceUI();
  validateWeeklySlots();
}

function getWeeklySlots() {
  if (!dom.weeklySlots) return [];
  const slots = [];
  dom.weeklySlots.querySelectorAll(".weekly-slot").forEach((slot) => {
    const day = Number(slot.querySelector('[name="slotDay"]').value);
    slots.push({
      day,
      defaultStart: slot.querySelector('[name="slotDefaultStart"]').value,
      defaultEnd: slot.querySelector('[name="slotDefaultEnd"]').value,
      schedStart: slot.querySelector('[name="slotSchedStart"]').value,
      schedEnd: slot.querySelector('[name="slotSchedEnd"]').value,
      name: slot.querySelector('[name="slotName"]').value,
      description: slot.querySelector('[name="slotDescription"]').value,
    });
  });
  return slots;
}

function validateWeeklySlots() {
  if (!dom.weeklySlotStatus) return true;
  const slots = getWeeklySlots();
  if (slots.length === 0) {
    dom.weeklySlotStatus.textContent = "Add at least one weekly slot.";
    return false;
  }
  const ranges = [];
  for (const slot of slots) {
    if (Number.isNaN(slot.day)) {
      dom.weeklySlotStatus.textContent = "Weekly slots need a day of week.";
      return false;
    }
    const defaultStart = timeToMinutes(slot.defaultStart);
    const defaultEnd = timeToMinutes(slot.defaultEnd);
    const schedStart = timeToMinutes(slot.schedStart);
    const schedEnd = timeToMinutes(slot.schedEnd);
    if (
      defaultStart === null ||
      defaultEnd === null ||
      schedStart === null ||
      schedEnd === null
    ) {
      dom.weeklySlotStatus.textContent = "Weekly slots need valid times.";
      return false;
    }
    if (defaultEnd <= defaultStart || schedEnd <= schedStart) {
      dom.weeklySlotStatus.textContent = "Weekly slots must end after they start.";
      return false;
    }
    if (schedStart > defaultStart || schedEnd < defaultEnd) {
      dom.weeklySlotStatus.textContent =
        "Schedulable range must contain default range for each slot.";
      return false;
    }
    const offset = dayOffsetFromMonday(slot.day);
    ranges.push({
      start: offset * 1440 + schedStart,
      end: offset * 1440 + schedEnd,
    });
  }
  const sorted = ranges.sort((a, b) => a.start - b.start);
  for (let i = 0; i < sorted.length - 1; i += 1) {
    if (sorted[i].end > sorted[i + 1].start) {
      dom.weeklySlotStatus.textContent = "Weekly slots cannot overlap.";
      return false;
    }
  }
  dom.weeklySlotStatus.textContent = "";
  return true;
}

function resetFormMode() {
  state.editingRecurrenceId = null;
  state.editingRecurrenceType = null;
  state.selectionMode = false;
  state.selectionStep = null;
  state.pendingDefaultRange = null;
  state.pendingSchedulableRange = null;
  state.selectionPointer = null;
  if (state.selectionScrollHandler) {
    window.removeEventListener("scroll", state.selectionScrollHandler);
    window.removeEventListener("resize", state.selectionScrollHandler);
    state.selectionScrollHandler = null;
  }
  dom.blobForm.reset();
  if (dom.recurrenceType) {
    dom.recurrenceType.value = "single";
  }
  clearWeeklySlots();
  createWeeklySlot();
  if (dom.weeklySlotStatus) {
    dom.weeklySlotStatus.textContent = "";
  }
  updateRecurrenceUI();
  setFormMode("create");
  dom.formStatus.textContent = "";
  document.querySelectorAll(".selection-overlay").forEach((overlay) => {
    overlay.classList.remove("active");
    overlay.style.top = "";
    overlay.style.height = "";
  });
  document.querySelectorAll(".selection-caret").forEach((caret) => {
    caret.classList.remove("active");
    caret.style.top = "";
  });
}

function openEditForm(blob) {
  state.selectionMode = false;
  state.selectionStep = null;
  state.pendingDefaultRange = null;
  state.pendingSchedulableRange = null;
  const recurrenceType = blob.recurrence_type || "single";
  if (dom.recurrenceType) {
    dom.recurrenceType.value = recurrenceType;
  }
  dom.blobForm.name.value = blob.name || "";
  dom.blobForm.description.value = blob.description || "";
  dom.blobForm.defaultStart.value = toLocalInputValue(blob.default_scheduled_timerange?.start);
  dom.blobForm.defaultEnd.value = toLocalInputValue(blob.default_scheduled_timerange?.end);
  dom.blobForm.schedulableStart.value = toLocalInputValue(blob.schedulable_timerange?.start);
  dom.blobForm.schedulableEnd.value = toLocalInputValue(blob.schedulable_timerange?.end);
  clearWeeklySlots();
  if (blob.recurrence_payload?.interval && dom.blobForm.weeklyInterval) {
    dom.blobForm.weeklyInterval.value = blob.recurrence_payload.interval;
  }
  if (recurrenceType === "weekly" && blob.recurrence_payload?.blobs_of_week) {
    const blobs = blob.recurrence_payload.blobs_of_week;
    const sharedName = blobs[0]?.name || "";
    const sharedDescription = blobs[0]?.description || "";
    dom.blobForm.name.value = sharedName;
    dom.blobForm.description.value = sharedDescription || "";
    const hasCustom =
      blobs.some((item) => item.name !== sharedName || item.description !== sharedDescription) ||
      false;
    if (dom.weeklyPerSlot) {
      dom.weeklyPerSlot.checked = hasCustom;
    }
    blobs.forEach((weeklyBlob) => {
      const start = new Date(weeklyBlob.default_scheduled_timerange?.start);
      const end = new Date(weeklyBlob.default_scheduled_timerange?.end);
      const schedStart = new Date(weeklyBlob.schedulable_timerange?.start);
      const schedEnd = new Date(weeklyBlob.schedulable_timerange?.end);
      const dayValue = Number.isNaN(start.getTime()) ? 1 : start.getDay();
      createWeeklySlot({
        day: dayValue,
        defaultStart: timeValueFromDate(start, "09:00"),
        defaultEnd: timeValueFromDate(end, "10:00"),
        schedStart: timeValueFromDate(schedStart, "08:30"),
        schedEnd: timeValueFromDate(schedEnd, "10:30"),
        name: weeklyBlob.name || "",
        description: weeklyBlob.description || "",
      });
    });
  } else {
    createWeeklySlot();
  }
  if (blob.recurrence_payload?.delta_seconds && dom.blobForm.deltaValue) {
    const deltaSeconds = Number(blob.recurrence_payload.delta_seconds);
    let value = Math.max(1, Math.round(deltaSeconds / 86400));
    let unit = "days";
    if (deltaSeconds % 604800 === 0) {
      value = Math.max(1, Math.round(deltaSeconds / 604800));
      unit = "weeks";
    } else if (deltaSeconds % 3600 === 0) {
      value = Math.max(1, Math.round(deltaSeconds / 3600));
      unit = "hours";
    } else if (deltaSeconds % 60 === 0) {
      value = Math.max(1, Math.round(deltaSeconds / 60));
      unit = "minutes";
    }
    dom.blobForm.deltaValue.value = value;
    dom.blobForm.deltaUnit.value = unit;
  }
  state.editingRecurrenceId = blob.recurrence_id || null;
  state.editingRecurrenceType = recurrenceType;
  updateRecurrenceUI();
  setFormMode("edit");
  dom.formStatus.textContent = "";
  toggleForm(true);
}

async function handleBlobSubmit(event) {
  event.preventDefault();
  dom.formStatus.textContent = "Saving...";
  const formData = new FormData(dom.blobForm);
  const baseBlob = {
    name: formData.get("name"),
    description: formData.get("description") || null,
    tz: getLocalTimeZone(),
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
  const recurrenceType = formData.get("recurrenceType") || "single";
  let recurrencePayload = {};
  if (recurrenceType === "weekly") {
    const isValid = validateWeeklySlots();
    if (!isValid) {
      dom.formStatus.textContent = "Fix weekly slot errors before saving.";
      return;
    }
    const weekStart = getWeekStart(state.anchorDate);
    const slots = getWeeklySlots();
    const perSlot = Boolean(dom.weeklyPerSlot?.checked);
    const sharedName = formData.get("name");
    const sharedDescription = formData.get("description") || null;
    const blobsOfWeek = slots.map((slot) => {
      const offset = dayOffsetFromMonday(slot.day);
      const slotDate = addDays(weekStart, offset);
      const defaultStart = timeToDate(slotDate, slot.defaultStart);
      const defaultEnd = timeToDate(slotDate, slot.defaultEnd);
      const schedStart = timeToDate(slotDate, slot.schedStart);
      const schedEnd = timeToDate(slotDate, slot.schedEnd);
      return {
        name: perSlot && slot.name ? slot.name : sharedName,
        description: perSlot ? slot.description || null : sharedDescription,
        tz: getLocalTimeZone(),
        default_scheduled_timerange: {
          start: toIso(defaultStart),
          end: toIso(defaultEnd),
        },
        schedulable_timerange: {
          start: toIso(schedStart),
          end: toIso(schedEnd),
        },
        policy: {},
        dependencies: [],
        tags: [],
      };
    });
    recurrencePayload = {
      interval: Math.max(1, Number(formData.get("weeklyInterval") || 1)),
      blobs_of_week: blobsOfWeek,
    };
  } else if (recurrenceType === "delta") {
    const value = Math.max(1, Number(formData.get("deltaValue") || 1));
    const unit = formData.get("deltaUnit") || "days";
    const unitSeconds = {
      minutes: 60,
      hours: 3600,
      days: 86400,
      weeks: 604800,
    };
    recurrencePayload = {
      delta_seconds: value * (unitSeconds[unit] || 86400),
      start_blob: baseBlob,
    };
  } else if (recurrenceType === "date") {
    recurrencePayload = { blob: baseBlob };
  } else {
    recurrencePayload = { blob: baseBlob };
  }
  const payload = { type: recurrenceType, payload: recurrencePayload };

  try {
    const isEditing = Boolean(state.editingRecurrenceId);
    const endpoint = isEditing
      ? `${API_BASE}/recurrences/${state.editingRecurrenceId}`
      : `${API_BASE}/recurrences`;
    const response = await fetch(endpoint, {
      method: isEditing ? "PUT" : "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });
    if (!response.ok) {
      let detail = "Failed to save recurrence";
      const contentType = response.headers.get("content-type") || "";
      if (contentType.includes("application/json")) {
        const data = await response.json();
        detail = data.detail || detail;
      } else {
        detail = (await response.text()) || detail;
      }
      throw new Error(detail);
    }
    dom.blobForm.reset();
    dom.formStatus.textContent = isEditing ? "Updated." : "Created.";
    toggleForm(false);
    resetFormMode();
  if (refreshView) {
      await refreshView(state.view);
    }
  } catch (error) {
    dom.formStatus.textContent = error?.message || "Error saving recurrence.";
  }
}

function handleSettingsSubmit(event) {
  event.preventDefault();
  const formData = new FormData(dom.settingsForm);
  const scheduleName = formData.get("scheduleName")?.toString().trim() || "";
  const subtitle = formData.get("subtitle")?.toString().trim() || "";
  const granularity = Math.max(1, Number(formData.get("minuteGranularity") || 1));
  appConfig.scheduleName = scheduleName || appConfig.scheduleName;
  appConfig.subtitle = subtitle || appConfig.subtitle;
  appConfig.minuteGranularity = granularity;
  dom.brandTitle.textContent = appConfig.scheduleName;
  dom.brandSubtitle.textContent = appConfig.subtitle;
  dom.settingsStatus.textContent = "Saved. Refresh to apply granularity.";
  saveSettings(appConfig);
}

function handleAddClick() {
  resetFormMode();
  toggleForm(true);
  startInteractiveCreate();
}

function handleSettingsClick() {
  toggleSettings(true);
  hydrateSettingsForm();
}

function handleCloseSettings() {
  toggleSettings(false);
  dom.settingsStatus.textContent = "";
}

function handleCloseForm() {
  toggleForm(false);
  resetFormMode();
}

function handlePrevDay() {
  state.anchorDate = addDays(state.anchorDate, -1);
  if (refreshView) {
    refreshView("day");
  }
}

function handleNextDay() {
  state.anchorDate = addDays(state.anchorDate, 1);
  if (refreshView) {
    refreshView("day");
  }
}

function handleToday() {
  state.anchorDate = new Date();
  if (refreshView) {
    refreshView("day");
  }
}

function bindFormHandlers(onRefresh) {
  setRefreshHandler(onRefresh);
  dom.blobForm.addEventListener("submit", handleBlobSubmit);
  dom.settingsForm.addEventListener("submit", handleSettingsSubmit);
  dom.toggleFormBtn.addEventListener("click", handleAddClick);
  dom.settingsBtn.addEventListener("click", handleSettingsClick);
  dom.closeSettingsBtn.addEventListener("click", handleCloseSettings);
  dom.settingsBackdrop.addEventListener("click", handleCloseSettings);
  dom.closeFormBtn.addEventListener("click", handleCloseForm);
  dom.prevDayBtn.addEventListener("click", handlePrevDay);
  dom.nextDayBtn.addEventListener("click", handleNextDay);
  dom.goTodayBtn.addEventListener("click", handleToday);
  if (dom.recurrenceType) {
    dom.recurrenceType.addEventListener("change", updateRecurrenceUI);
    dom.blobForm.defaultStart.addEventListener("change", updateRecurrenceUI);
    dom.blobForm.weeklyInterval.addEventListener("input", updateRecurrenceUI);
    dom.blobForm.deltaValue.addEventListener("input", updateRecurrenceUI);
    dom.blobForm.deltaUnit.addEventListener("change", updateRecurrenceUI);
  }
  if (dom.weeklyPerSlot) {
    dom.weeklyPerSlot.addEventListener("change", updateRecurrenceUI);
  }
  if (dom.addWeeklySlotBtn) {
    dom.addWeeklySlotBtn.addEventListener("click", () => createWeeklySlot());
  }
  if (dom.weeklySlots && dom.weeklySlots.children.length === 0) {
    createWeeklySlot();
  }
  updateRecurrenceUI();
}

export { bindFormHandlers, handleAddClick, openEditForm, resetFormMode, toggleForm, toggleSettings };
