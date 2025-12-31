import { API_BASE, appConfig, saveSettings, state } from "./core.js";
import { dom } from "./dom.js";
import { addDays, toIso, toLocalInputValue } from "./utils.js";
import { fetchBlobs } from "./api.js";
import { renderAll, setActive, startInteractiveCreate } from "./render.js";

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
    dom.formTitle.textContent = "Edit blob";
    dom.formSubmitBtn.textContent = "Update";
  } else {
    dom.formTitle.textContent = "Create a blob";
    dom.formSubmitBtn.textContent = "Create";
  }
}

function resetFormMode() {
  state.editingBlobId = null;
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
  dom.blobForm.name.value = blob.name || "";
  dom.blobForm.description.value = blob.description || "";
  dom.blobForm.defaultStart.value = toLocalInputValue(blob.default_scheduled_timerange?.start);
  dom.blobForm.defaultEnd.value = toLocalInputValue(blob.default_scheduled_timerange?.end);
  dom.blobForm.schedulableStart.value = toLocalInputValue(blob.schedulable_timerange?.start);
  dom.blobForm.schedulableEnd.value = toLocalInputValue(blob.schedulable_timerange?.end);
  state.editingBlobId = blob.id;
  setFormMode("edit");
  dom.formStatus.textContent = "";
  toggleForm(true);
}

async function handleBlobSubmit(event) {
  event.preventDefault();
  dom.formStatus.textContent = "Saving...";
  const formData = new FormData(dom.blobForm);
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
    dom.blobForm.reset();
    dom.formStatus.textContent = isEditing ? "Updated." : "Created.";
    toggleForm(false);
    resetFormMode();
    await fetchBlobs();
  } catch (error) {
    dom.formStatus.textContent = error?.message || "Error saving blob.";
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
  renderAll();
  setActive("day");
}

function handleNextDay() {
  state.anchorDate = addDays(state.anchorDate, 1);
  renderAll();
  setActive("day");
}

function handleToday() {
  state.anchorDate = new Date();
  renderAll();
  setActive("day");
}

function bindFormHandlers() {
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
}

export { bindFormHandlers, handleAddClick, openEditForm, resetFormMode, toggleForm, toggleSettings };
