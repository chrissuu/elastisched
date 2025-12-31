import { appConfig, isTypingInField, loadView, state } from "./core.js";
import { dom } from "./dom.js";
import { fetchBlobs } from "./api.js";
import { bindFormHandlers, openEditForm, resetFormMode, toggleForm, toggleSettings } from "./forms.js";
import { renderAll, setActive, startInteractiveCreate } from "./render.js";
import { addDays } from "./utils.js";

dom.brandTitle.textContent = appConfig.scheduleName || dom.brandTitle.textContent;
dom.brandSubtitle.textContent = appConfig.subtitle || dom.brandSubtitle.textContent;

bindFormHandlers();

dom.tabs.forEach((tab) => {
  tab.addEventListener("click", () => setActive(tab.dataset.view));
});

document.addEventListener("click", (event) => {
  const target = event.target.closest("[data-date]");
  if (!target) return;
  const dateIso = target.getAttribute("data-date");
  if (dateIso) {
    state.anchorDate = new Date(dateIso);
    renderAll();
    setActive("day");
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
    if (view === "day") {
      state.anchorDate = addDays(state.anchorDate, direction);
    } else if (view === "week") {
      state.anchorDate = addDays(state.anchorDate, direction * 7);
    } else if (view === "month") {
      const next = new Date(state.anchorDate);
      next.setMonth(next.getMonth() + direction);
      state.anchorDate = next;
    } else if (view === "year") {
      const next = new Date(state.anchorDate);
      next.setFullYear(next.getFullYear() + direction);
      state.anchorDate = next;
    } else {
      return;
    }
    event.preventDefault();
    renderAll();
    setActive(view);
  }
});

resetFormMode();
const savedView = loadView();
setActive(savedView || "day");
fetchBlobs();
