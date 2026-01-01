import { appConfig, isTypingInField, loadView, state } from "./core.js";
import { dom } from "./dom.js";
import { ensureOccurrences } from "./api.js";
import { bindFormHandlers, openEditForm, resetFormMode, toggleForm, toggleSettings } from "./forms.js";
import { setActive, startInteractiveCreate } from "./render.js";
import { getViewRange, shiftAnchorDate } from "./utils.js";

dom.brandTitle.textContent = appConfig.scheduleName || dom.brandTitle.textContent;
dom.brandSubtitle.textContent = appConfig.subtitle || dom.brandSubtitle.textContent;

bindFormHandlers(refreshView);

async function refreshView(nextView = state.view) {
  const view = nextView || state.view;
  setActive(view);
  const range = getViewRange(view, state.anchorDate);
  await ensureOccurrences(range.start, range.end);
  setActive(view);
}

dom.tabs.forEach((tab) => {
  tab.addEventListener("click", () => refreshView(tab.dataset.view));
});

document.addEventListener("click", (event) => {
  const target = event.target.closest("[data-date]");
  if (!target) return;
  const dateIso = target.getAttribute("data-date");
  if (!dateIso) return;
  state.anchorDate = new Date(dateIso);
  if (target.classList.contains("year-month")) {
    refreshView("month");
    return;
  }
  refreshView("day");
});

document.addEventListener("contextmenu", (event) => {
  const target = event.target.closest("[data-blob-id]");
  if (!target) return;
  if (dom.formPanel.classList.contains("active")) {
    return;
  }
  event.preventDefault();
  const blobId = target.getAttribute("data-blob-id");
  const blob = state.blobs.find((item) => item.id === blobId);
  if (blob) {
    openEditForm(blob);
  }
});

document.addEventListener("click", (event) => {
  if (!dom.formPanel.classList.contains("active")) return;
  if (!state.editingRecurrenceId) return;
  if (dom.formPanel.contains(event.target)) return;
  toggleForm(false);
  resetFormMode();
});

window.addEventListener("keydown", (event) => {
  if (isTypingInField(event.target)) return;
  const isArrowLeft =
    event.key === "ArrowLeft" ||
    event.key === "Left" ||
    event.code === "ArrowLeft" ||
    event.keyCode === 37;
  const isArrowRight =
    event.key === "ArrowRight" ||
    event.key === "Right" ||
    event.code === "ArrowRight" ||
    event.keyCode === 39;
  if (event.key === "Escape") {
    if (dom.settingsModal.classList.contains("active")) {
      toggleSettings(false);
      dom.settingsStatus.textContent = "";
    }
    if (dom.formPanel.classList.contains("active")) {
      toggleForm(false);
      resetFormMode();
    }
    return;
  }
  if (event.key.toLowerCase() === "n") {
    event.preventDefault();
    resetFormMode();
    toggleForm(true);
    startInteractiveCreate();
  }
  if (isArrowLeft || isArrowRight) {
    const direction = isArrowLeft ? -1 : 1;
    const view = state.view;
    const next = shiftAnchorDate(view, state.anchorDate, direction);
    if (!next) {
      return;
    }
    state.anchorDate = next;
    event.preventDefault();
    refreshView(view);
  }
});

resetFormMode();
const savedView = loadView();
refreshView(savedView || "day");
