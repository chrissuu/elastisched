import { API_BASE, appConfig, saveSettings, state } from "./core.js";
import { dom } from "./dom.js";
import {
  addDays,
  formatDateTimeLocalInTimeZone,
  getWeekStart,
  shiftAnchorDate,
  toLocalInputValueInTimeZone,
  toProjectIsoFromLocalInput,
} from "./utils.js";
import { startInteractiveCreate } from "./render.js";

let refreshView = null;
const recurrenceFieldGroups = document.querySelectorAll(".recurrence-fields");
const WEEK_DAYS = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
const editOnlyElements = document.querySelectorAll(".edit-only");

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
  dom.settingsForm.userTimeZone.value = appConfig.userTimeZone || "";
}

function setFormMode(mode) {
  if (mode === "edit") {
    dom.formTitle.textContent = "Edit recurrence";
    dom.formSubmitBtn.textContent = "Update";
    editOnlyElements.forEach((el) => {
      el.classList.add("active");
    });
  } else {
    dom.formTitle.textContent = "Create a recurrence";
    dom.formSubmitBtn.textContent = "Create";
    editOnlyElements.forEach((el) => {
      el.classList.remove("active");
    });
  }
}

async function refreshCalendar() {
  state.loadedRange = null;
  if (refreshView) {
    await refreshView(state.view);
  }
}

function getPolicyFlagsFromPolicy(policy = {}) {
  const rawMask = Number(policy.scheduling_policies);
  const mask = Number.isFinite(rawMask) ? rawMask : 0;
  const splittable =
    typeof policy.is_splittable === "boolean" ? policy.is_splittable : Boolean(mask & 1);
  const overlappable =
    typeof policy.is_overlappable === "boolean"
      ? policy.is_overlappable
      : Boolean(mask & 2);
  const invisible =
    typeof policy.is_invisible === "boolean" ? policy.is_invisible : Boolean(mask & 4);
  return { splittable, overlappable, invisible };
}

function getPolicyPayloadFromForm() {
  return getPolicyPayloadFromFlags(
    Boolean(dom.blobForm.policySplittable?.checked),
    Boolean(dom.blobForm.policyOverlappable?.checked),
    Boolean(dom.blobForm.policyInvisible?.checked)
  );
}

function getPolicyPayloadFromFlags(splittable, overlappable, invisible) {
  const schedulingPolicies =
    (splittable ? 1 : 0) | (overlappable ? 2 : 0) | (invisible ? 4 : 0);
  return {
    is_splittable: splittable,
    is_overlappable: overlappable,
    is_invisible: invisible,
    scheduling_policies: schedulingPolicies,
  };
}

function applyPolicyToForm(policy) {
  const flags = getPolicyFlagsFromPolicy(policy || {});
  if (dom.blobForm.policySplittable) {
    dom.blobForm.policySplittable.checked = flags.splittable;
  }
  if (dom.blobForm.policyOverlappable) {
    dom.blobForm.policyOverlappable.checked = flags.overlappable;
  }
  if (dom.blobForm.policyInvisible) {
    dom.blobForm.policyInvisible.checked = flags.invisible;
  }
}

function applyPolicyToSlot(slot, policy) {
  const flags = getPolicyFlagsFromPolicy(policy || {});
  const splittableEl = slot.querySelector('[name="slotPolicySplittable"]');
  const overlappableEl = slot.querySelector('[name="slotPolicyOverlappable"]');
  const invisibleEl = slot.querySelector('[name="slotPolicyInvisible"]');
  if (splittableEl) splittableEl.checked = flags.splittable;
  if (overlappableEl) overlappableEl.checked = flags.overlappable;
  if (invisibleEl) invisibleEl.checked = flags.invisible;
}

function syncSlotPoliciesFromForm() {
  if (!dom.weeklySlots) return;
  const sharedPolicy = getPolicyPayloadFromForm();
  dom.weeklySlots.querySelectorAll(".weekly-slot").forEach((slot) => {
    applyPolicyToSlot(slot, sharedPolicy);
  });
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
    const isCheckbox = field.type === "checkbox";
    const isOptional = field.name === "blobDescription";
    field.disabled = shouldDisable;
    field.required = !shouldDisable && !isCheckbox && !isOptional;
  });

  const defaultStart = dom.blobForm.defaultStart.value;
  const startDate = defaultStart
    ? new Date(
        toProjectIsoFromLocalInput(
          defaultStart,
          appConfig.userTimeZone,
          appConfig.projectTimeZone
        )
      )
    : state.anchorDate;
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

function timeValueFromDate(date, fallback, timeZone) {
  if (!date || Number.isNaN(date.getTime())) return fallback;
  const local = formatDateTimeLocalInTimeZone(date, timeZone);
  return local.split("T")[1] || fallback;
}

function weekdayIndexFromDateString(dateString) {
  const [year, month, day] = dateString.split("-").map((part) => Number(part));
  if ([year, month, day].some((item) => Number.isNaN(item))) {
    return 1;
  }
  return new Date(Date.UTC(year, month - 1, day)).getUTCDay();
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
  const fallbackPolicy = dom.weeklyPerSlot?.checked ? getPolicyPayloadFromForm() : {};
  const policyFlags = getPolicyFlagsFromPolicy(slotData.policy ?? fallbackPolicy);
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
    <div class="weekly-slot-row slot-policy">
      <label class="policy-option">
        <input type="checkbox" name="slotPolicySplittable" ${
          policyFlags.splittable ? "checked" : ""
        } />
        <span>Splittable</span>
      </label>
      <label class="policy-option">
        <input type="checkbox" name="slotPolicyOverlappable" ${
          policyFlags.overlappable ? "checked" : ""
        } />
        <span>Overlappable</span>
      </label>
      <label class="policy-option">
        <input type="checkbox" name="slotPolicyInvisible" ${
          policyFlags.invisible ? "checked" : ""
        } />
        <span>Invisible</span>
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
    const splittable = Boolean(slot.querySelector('[name="slotPolicySplittable"]')?.checked);
    const overlappable = Boolean(slot.querySelector('[name="slotPolicyOverlappable"]')?.checked);
    const invisible = Boolean(slot.querySelector('[name="slotPolicyInvisible"]')?.checked);
    slots.push({
      day,
      defaultStart: slot.querySelector('[name="slotDefaultStart"]').value,
      defaultEnd: slot.querySelector('[name="slotDefaultEnd"]').value,
      schedStart: slot.querySelector('[name="slotSchedStart"]').value,
      schedEnd: slot.querySelector('[name="slotSchedEnd"]').value,
      name: slot.querySelector('[name="slotName"]').value,
      description: slot.querySelector('[name="slotDescription"]').value,
      policy: getPolicyPayloadFromFlags(splittable, overlappable, invisible),
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
  state.editingRecurrencePayload = null;
  state.editingOccurrenceStart = null;
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
  applyPolicyToForm({});
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
  dom.blobForm.recurrenceName.value = blob.recurrence_payload?.recurrence_name || "";
  dom.blobForm.recurrenceDescription.value =
    blob.recurrence_payload?.recurrence_description || "";
  dom.blobForm.blobName.value = blob.name || "";
  dom.blobForm.blobDescription.value = blob.description || "";
  dom.blobForm.defaultStart.value = toLocalInputValueInTimeZone(
    blob.default_scheduled_timerange?.start,
    appConfig.userTimeZone
  );
  dom.blobForm.defaultEnd.value = toLocalInputValueInTimeZone(
    blob.default_scheduled_timerange?.end,
    appConfig.userTimeZone
  );
  dom.blobForm.schedulableStart.value = toLocalInputValueInTimeZone(
    blob.schedulable_timerange?.start,
    appConfig.userTimeZone
  );
  dom.blobForm.schedulableEnd.value = toLocalInputValueInTimeZone(
    blob.schedulable_timerange?.end,
    appConfig.userTimeZone
  );
  clearWeeklySlots();
  if (blob.recurrence_payload?.interval && dom.blobForm.weeklyInterval) {
    dom.blobForm.weeklyInterval.value = blob.recurrence_payload.interval;
  }
  if (recurrenceType === "weekly" && blob.recurrence_payload?.blobs_of_week) {
    const blobs = blob.recurrence_payload.blobs_of_week;
    applyPolicyToForm(blobs[0]?.policy || {});
    const sharedName = blobs[0]?.name || "";
    const sharedDescription = blobs[0]?.description || "";
    dom.blobForm.blobName.value = sharedName;
    dom.blobForm.blobDescription.value = sharedDescription || "";
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
      const startLocal = Number.isNaN(start.getTime())
        ? ""
        : formatDateTimeLocalInTimeZone(start, appConfig.userTimeZone);
      const dayValue = startLocal
        ? weekdayIndexFromDateString(startLocal.split("T")[0])
        : 1;
      createWeeklySlot({
        day: dayValue,
        defaultStart: timeValueFromDate(start, "09:00", appConfig.userTimeZone),
        defaultEnd: timeValueFromDate(end, "10:00", appConfig.userTimeZone),
        schedStart: timeValueFromDate(schedStart, "08:30", appConfig.userTimeZone),
        schedEnd: timeValueFromDate(schedEnd, "10:30", appConfig.userTimeZone),
        name: weeklyBlob.name || "",
        description: weeklyBlob.description || "",
        policy: weeklyBlob.policy || {},
      });
    });
  } else {
    applyPolicyToForm(blob.policy);
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
  state.editingRecurrencePayload = blob.recurrence_payload || {};
  state.editingOccurrenceStart = blob.schedulable_timerange?.start || null;
  updateRecurrenceUI();
  setFormMode("edit");
  dom.formStatus.textContent = "";
  toggleForm(true);
}

async function deleteRecurrence() {
  if (!state.editingRecurrenceId) return;
  const confirmed = window.confirm("Delete this entire recurrence?");
  if (!confirmed) return;
  dom.formStatus.textContent = "Deleting recurrence...";
  try {
    const response = await fetch(`${API_BASE}/recurrences/${state.editingRecurrenceId}`, {
      method: "DELETE",
    });
    if (!response.ok) {
      throw new Error("Failed to delete recurrence");
    }
    dom.formStatus.textContent = "Deleted.";
    toggleForm(false);
    resetFormMode();
    await refreshCalendar();
  } catch (error) {
    dom.formStatus.textContent = error?.message || "Error deleting recurrence.";
  }
}

async function deleteOccurrence() {
  if (!state.editingRecurrenceId) return;
  if (state.editingRecurrenceType === "single") {
    await deleteRecurrence();
    return;
  }
  const occurrenceStart = state.editingOccurrenceStart;
  if (!occurrenceStart) {
    dom.formStatus.textContent = "Missing occurrence start.";
    return;
  }
  const confirmed = window.confirm("Delete only this occurrence?");
  if (!confirmed) return;
  const existing = Array.isArray(state.editingRecurrencePayload?.exclusions)
    ? state.editingRecurrencePayload.exclusions
    : [];
  const nextExclusions = Array.from(new Set([...existing, occurrenceStart]));
  const payload = {
    type: state.editingRecurrenceType,
    payload: { ...state.editingRecurrencePayload, exclusions: nextExclusions },
  };
  dom.formStatus.textContent = "Deleting occurrence...";
  try {
    const response = await fetch(`${API_BASE}/recurrences/${state.editingRecurrenceId}`, {
      method: "PUT",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });
    if (!response.ok) {
      throw new Error("Failed to delete occurrence");
    }
    dom.formStatus.textContent = "Deleted.";
    toggleForm(false);
    resetFormMode();
    await refreshCalendar();
  } catch (error) {
    dom.formStatus.textContent = error?.message || "Error deleting occurrence.";
  }
}

async function handleBlobSubmit(event) {
  event.preventDefault();
  dom.formStatus.textContent = "Saving...";
  const formData = new FormData(dom.blobForm);
  const policyPayload = getPolicyPayloadFromForm();
  const baseBlob = {
    name: formData.get("blobName"),
    description: formData.get("blobDescription") || null,
    tz: appConfig.userTimeZone,
    default_scheduled_timerange: {
      start: toProjectIsoFromLocalInput(
        formData.get("defaultStart"),
        appConfig.userTimeZone,
        appConfig.projectTimeZone
      ),
      end: toProjectIsoFromLocalInput(
        formData.get("defaultEnd"),
        appConfig.userTimeZone,
        appConfig.projectTimeZone
      ),
    },
    schedulable_timerange: {
      start: toProjectIsoFromLocalInput(
        formData.get("schedulableStart"),
        appConfig.userTimeZone,
        appConfig.projectTimeZone
      ),
      end: toProjectIsoFromLocalInput(
        formData.get("schedulableEnd"),
        appConfig.userTimeZone,
        appConfig.projectTimeZone
      ),
    },
    policy: policyPayload,
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
    const sharedName = formData.get("blobName");
    const sharedDescription = formData.get("blobDescription") || null;
    const sharedPolicy = policyPayload;
    const blobsOfWeek = slots.map((slot) => {
      const offset = dayOffsetFromMonday(slot.day);
      const slotDate = addDays(weekStart, offset);
      const slotDateValue = formatDateTimeLocalInTimeZone(
        slotDate,
        appConfig.userTimeZone
      ).split("T")[0];
      const defaultStart = toProjectIsoFromLocalInput(
        `${slotDateValue}T${slot.defaultStart}`,
        appConfig.userTimeZone,
        appConfig.projectTimeZone
      );
      const defaultEnd = toProjectIsoFromLocalInput(
        `${slotDateValue}T${slot.defaultEnd}`,
        appConfig.userTimeZone,
        appConfig.projectTimeZone
      );
      const schedStart = toProjectIsoFromLocalInput(
        `${slotDateValue}T${slot.schedStart}`,
        appConfig.userTimeZone,
        appConfig.projectTimeZone
      );
      const schedEnd = toProjectIsoFromLocalInput(
        `${slotDateValue}T${slot.schedEnd}`,
        appConfig.userTimeZone,
        appConfig.projectTimeZone
      );
      return {
        name: perSlot && slot.name ? slot.name : sharedName,
        description: perSlot ? slot.description || null : sharedDescription,
        tz: appConfig.userTimeZone,
        default_scheduled_timerange: {
          start: defaultStart,
          end: defaultEnd,
        },
        schedulable_timerange: {
          start: schedStart,
          end: schedEnd,
        },
        policy: perSlot ? slot.policy : sharedPolicy,
        dependencies: [],
        tags: [],
      };
    });
    recurrencePayload = {
      interval: Math.max(1, Number(formData.get("weeklyInterval") || 1)),
      recurrence_name: formData.get("recurrenceName") || null,
      recurrence_description: formData.get("recurrenceDescription") || null,
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
      recurrence_name: formData.get("recurrenceName") || null,
      recurrence_description: formData.get("recurrenceDescription") || null,
      start_blob: baseBlob,
    };
  } else if (recurrenceType === "date") {
    recurrencePayload = {
      recurrence_name: formData.get("recurrenceName") || null,
      recurrence_description: formData.get("recurrenceDescription") || null,
      blob: baseBlob,
    };
  } else {
    recurrencePayload = {
      recurrence_name: formData.get("recurrenceName") || null,
      recurrence_description: formData.get("recurrenceDescription") || null,
      blob: baseBlob,
    };
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
    await refreshCalendar();
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
  const userTimeZone = formData.get("userTimeZone")?.toString().trim() || "";
  if (userTimeZone) {
    try {
      Intl.DateTimeFormat("en-US", { timeZone: userTimeZone });
    } catch (error) {
      dom.settingsStatus.textContent = "Invalid timezone. Use an IANA name.";
      return;
    }
  }
  appConfig.scheduleName = scheduleName || appConfig.scheduleName;
  appConfig.subtitle = subtitle || appConfig.subtitle;
  appConfig.minuteGranularity = granularity;
  if (userTimeZone) {
    appConfig.userTimeZone = userTimeZone;
  }
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

function getActiveView() {
  const activeViewEntry = Object.entries(dom.views).find(([, el]) => el.classList.contains("active"));
  if (activeViewEntry) {
    return activeViewEntry[0];
  }
  const activeTab = Array.from(dom.tabs).find((tab) => tab.classList.contains("active"));
  return activeTab?.dataset.view || state.view;
}

function handlePrevDay() {
  const view = getActiveView();
  const next = shiftAnchorDate(view, state.anchorDate, -1);
  if (!next) return;
  state.anchorDate = next;
  if (refreshView) {
    refreshView(view);
  }
}

function handleNextDay() {
  const view = getActiveView();
  const next = shiftAnchorDate(view, state.anchorDate, 1);
  if (!next) return;
  state.anchorDate = next;
  if (refreshView) {
    refreshView(view);
  }
}

function handleToday() {
  const view = getActiveView();
  state.anchorDate = new Date();
  if (refreshView) {
    refreshView(view);
  }
}

function bindFormHandlers(onRefresh) {
  setRefreshHandler(onRefresh);
  dom.blobForm.addEventListener("submit", handleBlobSubmit);
  dom.settingsForm.addEventListener("submit", handleSettingsSubmit);
  if (dom.deleteRecurrenceBtn) {
    dom.deleteRecurrenceBtn.addEventListener("click", deleteRecurrence);
  }
  if (dom.deleteOccurrenceBtn) {
    dom.deleteOccurrenceBtn.addEventListener("click", deleteOccurrence);
  }
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
    dom.weeklyPerSlot.addEventListener("change", () => {
      const weeklyWrapper = dom.weeklySlots?.closest(".weekly-slots");
      const wasPerSlot = weeklyWrapper?.classList.contains("per-slot");
      updateRecurrenceUI();
      if (!wasPerSlot && dom.weeklyPerSlot.checked) {
        syncSlotPoliciesFromForm();
      }
    });
  }
  if (dom.addWeeklySlotBtn) {
    dom.addWeeklySlotBtn.addEventListener("click", () => createWeeklySlot());
  }
  ["policySplittable", "policyOverlappable", "policyInvisible"].forEach((name) => {
    const field = dom.blobForm?.[name];
    if (field) {
      field.addEventListener("change", () => {
        if (!dom.weeklyPerSlot?.checked) {
          syncSlotPoliciesFromForm();
        }
      });
    }
  });
  if (dom.weeklySlots && dom.weeklySlots.children.length === 0) {
    createWeeklySlot();
  }
  updateRecurrenceUI();
}

export { bindFormHandlers, handleAddClick, openEditForm, resetFormMode, toggleForm, toggleSettings };
