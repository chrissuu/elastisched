const state = {
  blobs: [],
  view: "day",
  anchorDate: new Date(),
  editingBlobId: null,
  selectionMode: false,
  selectionStep: null,
  pendingDefaultRange: null,
  pendingSchedulableRange: null,
  selectionPointer: null,
  selectionScrollHandler: null,
};

const defaultConfig = {
  scheduleName: window.APP_CONFIG?.scheduleName || "Elastisched",
  subtitle: window.APP_CONFIG?.subtitle || "Schedule at a glance",
  minuteGranularity: Math.max(1, Number(window.APP_CONFIG?.minuteGranularity || 5)),
};

const storedConfig = (() => {
  try {
    const raw = window.localStorage.getItem("elastisched:settings");
    return raw ? JSON.parse(raw) : null;
  } catch (error) {
    return null;
  }
})();

const appConfig = {
  ...defaultConfig,
  ...(storedConfig || {}),
};

const minuteGranularity = Math.max(1, Number(appConfig.minuteGranularity || 5));
const API_BASE = window.location.origin;

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

function saveView(view) {
  try {
    window.localStorage.setItem("elastisched:view", view);
  } catch (error) {
    // Ignore storage errors.
  }
}

function loadView() {
  try {
    return window.localStorage.getItem("elastisched:view");
  } catch (error) {
    return null;
  }
}

function saveSettings(config) {
  try {
    window.localStorage.setItem("elastisched:settings", JSON.stringify(config));
  } catch (error) {
    // Ignore storage errors.
  }
}

function isTypingInField(target) {
  return (
    target &&
    (target.tagName === "INPUT" ||
      target.tagName === "TEXTAREA" ||
      target.isContentEditable)
  );
}

export {
  API_BASE,
  appConfig,
  demoBlobs,
  isTypingInField,
  loadView,
  minuteGranularity,
  saveSettings,
  saveView,
  state,
};
