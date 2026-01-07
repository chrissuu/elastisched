import { dom } from "./dom.js";

let resolver = null;
let actionMap = { confirm: true, cancel: false, alt: null };

function setModalActive(active) {
  if (!dom.alertModal || !dom.alertPanel) return;
  dom.alertModal.classList.toggle("active", active);
  dom.alertPanel.classList.toggle("active", active);
  dom.alertModal.setAttribute("aria-hidden", (!active).toString());
  document.body.classList.toggle("modal-open", active);
}

function closeDialog(result) {
  if (!resolver) {
    setModalActive(false);
    return;
  }
  const resolve = resolver;
  resolver = null;
  actionMap = { confirm: true, cancel: false, alt: null };
  setModalActive(false);
  resolve(result);
}

function configureDialog({
  title,
  message,
  confirmText,
  cancelText,
  altText,
  confirmVariant,
  altVariant,
  destructive = false,
  altDestructive = false,
  actionOrder = "cancel-alt-confirm",
}) {
  if (dom.alertTitle) dom.alertTitle.textContent = title || "Notice";
  if (dom.alertMessage) dom.alertMessage.textContent = message || "";
  if (dom.alertConfirmBtn) {
    dom.alertConfirmBtn.textContent = confirmText || "OK";
  }
  if (dom.alertCancelBtn) {
    if (cancelText) {
      dom.alertCancelBtn.textContent = cancelText;
      dom.alertCancelBtn.style.display = "";
    } else {
      dom.alertCancelBtn.style.display = "none";
    }
  }
  if (dom.alertAltBtn) {
    if (altText) {
      dom.alertAltBtn.textContent = altText;
      dom.alertAltBtn.style.display = "";
    } else {
      dom.alertAltBtn.style.display = "none";
    }
  }
  if (dom.alertConfirmBtn) {
    dom.alertConfirmBtn.classList.toggle("danger", destructive);
    dom.alertConfirmBtn.classList.toggle("ghost", confirmVariant === "ghost");
    dom.alertConfirmBtn.classList.toggle("primary", confirmVariant !== "ghost");
  }
  if (dom.alertAltBtn) {
    dom.alertAltBtn.classList.toggle("danger", altDestructive);
    dom.alertAltBtn.classList.toggle("ghost", altVariant !== "primary");
    dom.alertAltBtn.classList.toggle("primary", altVariant === "primary");
  }
  if (dom.alertPanel) {
    dom.alertPanel.dataset.actionOrder = actionOrder;
  }
}

function showDialog(options, actions) {
  if (resolver) {
    closeDialog(false);
  }
  configureDialog(options);
  if (actions) {
    actionMap = actions;
  }
  setModalActive(true);
  return new Promise((resolve) => {
    resolver = resolve;
  });
}

function confirmDialog(message, options = {}) {
  return showDialog(
    {
      title: options.title || "Confirm",
      message,
      confirmText: options.confirmText || "Confirm",
      cancelText: options.cancelText || "Cancel",
      destructive: Boolean(options.destructive),
    },
    { confirm: true, cancel: false, alt: false }
  );
}

function alertDialog(message, options = {}) {
  return showDialog(
    {
      title: options.title || "Notice",
      message,
      confirmText: options.confirmText || "OK",
      cancelText: null,
    },
    { confirm: true, cancel: true, alt: true }
  );
}

function choiceDialog(message, options = {}) {
  return showDialog(
    {
      title: options.title || "Choose an action",
      message,
      confirmText: options.confirmText || "Option A",
      altText: options.altText || "Option B",
      cancelText: options.cancelText || "Cancel",
      destructive: Boolean(options.destructive),
      altDestructive: Boolean(options.altDestructive),
      confirmVariant: options.confirmVariant,
      altVariant: options.altVariant,
      actionOrder: options.actionOrder || "cancel-alt-confirm",
    },
    {
      confirm: options.confirmValue ?? "confirm",
      alt: options.altValue ?? "alt",
      cancel: options.cancelValue ?? null,
    }
  );
}

function bindDialogEvents() {
  if (dom.alertConfirmBtn) {
    dom.alertConfirmBtn.addEventListener("click", () => closeDialog(actionMap.confirm));
  }
  if (dom.alertCancelBtn) {
    dom.alertCancelBtn.addEventListener("click", () => closeDialog(actionMap.cancel));
  }
  if (dom.alertAltBtn) {
    dom.alertAltBtn.addEventListener("click", () => closeDialog(actionMap.alt));
  }
  if (dom.alertBackdrop) {
    dom.alertBackdrop.addEventListener("click", () => closeDialog(actionMap.cancel));
  }
  window.addEventListener("keydown", (event) => {
    if (!resolver) return;
    if (event.key === "Escape") {
      closeDialog(actionMap.cancel);
    }
  });
}

export { alertDialog, bindDialogEvents, choiceDialog, confirmDialog };
