import { appConfig, isTypingInField, loadView, state } from "./core.js";
import { dom } from "./dom.js";
import { fetchBlobs } from "./api.js";
import { bindFormHandlers, openEditForm, resetFormMode, toggleForm, toggleSettings } from "./forms.js";
import { renderAll, setActive, startInteractiveCreate } from "./render.js";

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
});

resetFormMode();
const savedView = loadView();
setActive(savedView || "day");
fetchBlobs();
