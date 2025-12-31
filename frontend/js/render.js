import { appConfig, minuteGranularity, saveView, state } from "./core.js";
import { dom } from "./dom.js";
import {
  addDays,
  clampToGranularity,
  formatTimeRangeInTimeZone,
  getTagType,
  layoutBlocks,
  overlaps,
  startOfDay,
  toDate,
  toZonedDate,
  toLocalInputFromDate,
} from "./utils.js";

function setDateLabel(text) {
  dom.dateLabel.textContent = text;
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

function updateSelectionOverlay(overlayEl, startMin, endMin, hourHeight) {
  const top = (startMin / 60) * hourHeight;
  const height = Math.max(12, ((endMin - startMin) / 60) * hourHeight);
  overlayEl.style.top = `${top}px`;
  overlayEl.style.height = `${height}px`;
  overlayEl.classList.add("active");
}

function updateCaret(caretEl, minutes, hourHeight) {
  const top = (minutes / 60) * hourHeight;
  caretEl.style.top = `${top}px`;
  caretEl.classList.add("active");
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
      const start = toZonedDate(
        toDate(
        blob.realized_timerange?.start || blob.default_scheduled_timerange?.start
      ),
        appConfig.userTimeZone
      );
      const end = toZonedDate(
        toDate(
        blob.realized_timerange?.end || blob.default_scheduled_timerange?.end
      ),
        appConfig.userTimeZone
      );
      const schedStart = toZonedDate(
        toDate(blob.schedulable_timerange?.start),
        appConfig.userTimeZone
      );
      const schedEnd = toZonedDate(
        toDate(blob.schedulable_timerange?.end),
        appConfig.userTimeZone
      );
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
        time: formatTimeRangeInTimeZone(
          blob.realized_timerange?.start || blob.default_scheduled_timerange.start,
          blob.realized_timerange?.end || blob.default_scheduled_timerange.end,
          appConfig.userTimeZone
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

  dom.views.day.innerHTML = `
    <div class="day-grid" style="--hour-height: ${hourHeight}px;">
      <div class="hours">${hoursHtml}</div>
      <div class="day-track">
        <div class="schedulable-overlay" id="schedulableOverlay"></div>
        <div class="selection-overlay default-range" id="selectionOverlayDefault"></div>
        <div class="selection-overlay schedulable-range" id="selectionOverlaySchedulable"></div>
        <div class="selection-caret default-range" id="selectionCaretDefault"></div>
        <div class="selection-caret schedulable-range" id="selectionCaretSchedulable"></div>
        ${blockHtml || "<div class='day-empty'>No events yet</div>"}
      </div>
    </div>
  `;

  const overlay = dom.views.day.querySelector("#schedulableOverlay");
  const dayTrack = dom.views.day.querySelector(".day-track");
  const selectionOverlayDefault = dom.views.day.querySelector("#selectionOverlayDefault");
  const selectionOverlaySchedulable = dom.views.day.querySelector(
    "#selectionOverlaySchedulable"
  );
  const selectionCaretDefault = dom.views.day.querySelector("#selectionCaretDefault");
  const selectionCaretSchedulable = dom.views.day.querySelector(
    "#selectionCaretSchedulable"
  );
  const blocksEls = dom.views.day.querySelectorAll(".day-block");

  blocksEls.forEach((blockEl) => {
    blockEl.addEventListener("mouseenter", () => {
      blockEl.classList.add("hovered");
      const schedStart = toZonedDate(
        toDate(blockEl.getAttribute("data-sched-start")),
        appConfig.userTimeZone
      );
      const schedEnd = toZonedDate(
        toDate(blockEl.getAttribute("data-sched-end")),
        appConfig.userTimeZone
      );
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
    const trackMinutes = 24 * 60;

    const toMinutes = (clientY) => {
      const rect = dayTrack.getBoundingClientRect();
      const y = Math.min(Math.max(clientY - rect.top, 0), rect.height);
      return clampToGranularity(Math.round((y / rect.height) * trackMinutes));
    };

    const finalizeRange = (startMin, endMin) => {
      const startDate = new Date(dayStart.getTime() + startMin * 60000);
      const endDate = new Date(dayStart.getTime() + endMin * 60000);
      if (state.selectionStep === "default") {
        state.pendingDefaultRange = { start: startDate, end: endDate };
        state.selectionStep = "schedulable";
        dom.formStatus.textContent = "Click start/end for schedulable range.";
        selectionCaretDefault.classList.remove("active");
      } else if (state.selectionStep === "schedulable") {
        state.pendingSchedulableRange = { start: startDate, end: endDate };
        state.selectionMode = false;
        state.selectionStep = null;
        selectionOverlayDefault.classList.add("active");
        selectionOverlaySchedulable.classList.add("active");
        selectionCaretSchedulable.classList.remove("active");
        const defaultRange = state.pendingDefaultRange;
        const schedRange = state.pendingSchedulableRange;
        if (defaultRange && schedRange) {
          dom.blobForm.defaultStart.value = toLocalInputFromDate(defaultRange.start);
          dom.blobForm.defaultEnd.value = toLocalInputFromDate(defaultRange.end);
          dom.blobForm.schedulableStart.value = toLocalInputFromDate(schedRange.start);
          dom.blobForm.schedulableEnd.value = toLocalInputFromDate(schedRange.end);
          dom.blobForm.defaultStart.dispatchEvent(new Event("change", { bubbles: true }));
        }
        dom.formStatus.textContent = "Ranges captured. Fill details and create.";
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
        const caretEl =
          state.selectionStep === "default"
            ? selectionCaretDefault
            : selectionCaretSchedulable;
        const endMin = Math.min(trackMinutes, minutes + minuteGranularity);
        updateSelectionOverlay(overlayEl, minutes, endMin, hourHeight);
        updateCaret(caretEl, minutes, hourHeight);
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
        updateSelectionOverlay(overlayEl, startMin, endMin, hourHeight);
        finalizeRange(startMin, endMin);
        clickStart = null;
      }
    };

    const onMouseMove = (event) => {
      if (state.selectionStep === null) return;
      const minutes = toMinutes(event.clientY);
      const overlayEl =
        state.selectionStep === "default"
          ? selectionOverlayDefault
          : selectionOverlaySchedulable;
      const caretEl =
        state.selectionStep === "default"
          ? selectionCaretDefault
          : selectionCaretSchedulable;
      if (clickStart === null) {
        updateCaret(caretEl, minutes, hourHeight);
      } else {
        const startMin = Math.min(clickStart, minutes);
        const endMin = Math.min(
          trackMinutes,
          Math.max(clickStart + minuteGranularity, minutes)
        );
        updateSelectionOverlay(overlayEl, startMin, endMin, hourHeight);
        updateCaret(caretEl, clickStart, hourHeight);
      }
      state.selectionPointer = { x: event.clientX, y: event.clientY };
    };

    dayTrack.addEventListener("click", onClick);
    dayTrack.addEventListener("mousemove", onMouseMove);
    if (state.selectionScrollHandler) {
      window.removeEventListener("scroll", state.selectionScrollHandler);
      window.removeEventListener("resize", state.selectionScrollHandler);
    }
    state.selectionScrollHandler = () => {
      if (state.selectionStep === null) return;
      if (!state.selectionPointer) return;
      const minutes = toMinutes(state.selectionPointer.y);
      const overlayEl =
        state.selectionStep === "default"
          ? selectionOverlayDefault
          : selectionOverlaySchedulable;
      const caretEl =
        state.selectionStep === "default"
          ? selectionCaretDefault
          : selectionCaretSchedulable;
      if (clickStart === null) {
        updateCaret(caretEl, minutes, hourHeight);
      } else {
        const startMin = Math.min(clickStart, minutes);
        const endMin = Math.min(
          trackMinutes,
          Math.max(clickStart + minuteGranularity, minutes)
        );
        updateSelectionOverlay(overlayEl, startMin, endMin, hourHeight);
        updateCaret(caretEl, clickStart, hourHeight);
      }
    };
    window.addEventListener("scroll", state.selectionScrollHandler, { passive: true });
    window.addEventListener("resize", state.selectionScrollHandler);
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
          const start = toZonedDate(
            toDate(
            blob.realized_timerange?.start || blob.default_scheduled_timerange?.start
          ),
            appConfig.userTimeZone
          );
          const end = toZonedDate(
            toDate(
            blob.realized_timerange?.end || blob.default_scheduled_timerange?.end
          ),
            appConfig.userTimeZone
          );
          const schedStart = toZonedDate(
            toDate(blob.schedulable_timerange?.start),
            appConfig.userTimeZone
          );
          const schedEnd = toZonedDate(
            toDate(blob.schedulable_timerange?.end),
            appConfig.userTimeZone
          );
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
            time: formatTimeRangeInTimeZone(
              blob.realized_timerange?.start || blob.default_scheduled_timerange.start,
              blob.realized_timerange?.end || blob.default_scheduled_timerange.end,
              appConfig.userTimeZone
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
            <div class="selection-overlay default-range"></div>
            <div class="selection-overlay schedulable-range"></div>
            <div class="selection-caret default-range"></div>
            <div class="selection-caret schedulable-range"></div>
            ${blockHtml || "<div class='day-empty'>No events yet</div>"}
          </div>
        </div>
      `;
    })
    .join("");

  const hoursHtml = hours.map((hour) => `<div class="hour">${hour}</div>`).join("");
  dom.views.week.innerHTML = `
    <div class="week-timeline" style="--hour-height: ${hourHeight}px;">
      <div class="week-hours">${hoursHtml}</div>
      <div class="week-days">${columns}</div>
    </div>
  `;

  const weekTimeline = dom.views.week.querySelector(".week-timeline");
  const weekColumn = dom.views.week.querySelector(".week-day-column");
  const weekLabel = dom.views.week.querySelector(".week-day-label");
  if (weekTimeline && weekColumn && weekLabel) {
    const labelHeight = weekLabel.getBoundingClientRect().height;
    const style = getComputedStyle(weekColumn);
    const gap = parseFloat(style.rowGap || style.gap || "0");
    weekTimeline.style.setProperty("--week-label-offset", `${labelHeight + gap}px`);
  }

  const weekStart = startOfDay(monday);
  const weekEnd = addDays(weekStart, 7);
  const dayColumns = Array.from(dom.views.week.querySelectorAll(".week-day-column"));
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
        const schedStart = toZonedDate(
          toDate(blockEl.getAttribute("data-sched-start")),
          appConfig.userTimeZone
        );
        const schedEnd = toZonedDate(
          toDate(blockEl.getAttribute("data-sched-end")),
          appConfig.userTimeZone
        );
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
          overlay.classList.toggle(
            "overflow-top",
            schedStart < weekStart && dayStart.getTime() === weekStart.getTime()
          );
          overlay.classList.toggle(
            "overflow-bottom",
            schedEnd > weekEnd && dayEnd.getTime() === weekEnd.getTime()
          );
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

  if (state.selectionMode) {
    let clickStart = null;
    let activeColumnIndex = null;
    const trackMinutes = 24 * 60;

    const toMinutes = (clientY, track) => {
      const rect = track.getBoundingClientRect();
      const y = Math.min(Math.max(clientY - rect.top, 0), rect.height);
      return clampToGranularity(Math.round((y / rect.height) * trackMinutes));
    };

    const clearSelectionOverlays = (overlaySelector) => {
      dayColumns.forEach((column) => {
        const overlay = column.querySelector(overlaySelector);
        overlay.classList.remove("active");
      });
    };

    const clearSelectionCarets = (caretSelector) => {
      dayColumns.forEach((column) => {
        const caret = column.querySelector(caretSelector);
        caret.classList.remove("active");
      });
    };

    const updateSelectionRange = (startCol, startMin, endCol, endMin, overlaySelector) => {
      const rangeStart = Math.min(startCol, endCol);
      const rangeEnd = Math.max(startCol, endCol);
      clearSelectionOverlays(overlaySelector);
      for (let index = rangeStart; index <= rangeEnd; index += 1) {
        const overlay = dayColumns[index].querySelector(overlaySelector);
        const isStart = index === startCol;
        const isEnd = index === endCol;
        const rangeStartMin = isStart ? startMin : 0;
        const rangeEndMin = isEnd ? endMin : trackMinutes;
        const normalizedStart = Math.min(rangeStartMin, rangeEndMin);
        const normalizedEnd = Math.max(
          normalizedStart + minuteGranularity,
          rangeEndMin
        );
        updateSelectionOverlay(
          overlay,
          Math.min(normalizedStart, trackMinutes - minuteGranularity),
          Math.min(normalizedEnd, trackMinutes),
          hourHeight
        );
      }
    };

    const finalizeRange = (startCol, startMin, endCol, endMin) => {
      const rangeStartCol = Math.min(startCol, endCol);
      const rangeEndCol = Math.max(startCol, endCol);
      const rangeStartMin = startCol <= endCol ? startMin : endMin;
      const rangeEndMin = startCol <= endCol ? endMin : startMin;
      const startDay = startOfDay(days[rangeStartCol]);
      const endDay = startOfDay(days[rangeEndCol]);
      const startDate = new Date(startDay.getTime() + rangeStartMin * 60000);
      const endDate = new Date(endDay.getTime() + rangeEndMin * 60000);
      if (state.selectionStep === "default") {
        state.pendingDefaultRange = { start: startDate, end: endDate };
        state.selectionStep = "schedulable";
        dom.formStatus.textContent = "Click start/end for schedulable range.";
        clearSelectionCarets(".selection-caret.default-range");
      } else if (state.selectionStep === "schedulable") {
        state.pendingSchedulableRange = { start: startDate, end: endDate };
        state.selectionMode = false;
        state.selectionStep = null;
        const defaultRange = state.pendingDefaultRange;
        const schedRange = state.pendingSchedulableRange;
        if (defaultRange && schedRange) {
          dom.blobForm.defaultStart.value = toLocalInputFromDate(defaultRange.start);
          dom.blobForm.defaultEnd.value = toLocalInputFromDate(defaultRange.end);
          dom.blobForm.schedulableStart.value = toLocalInputFromDate(schedRange.start);
          dom.blobForm.schedulableEnd.value = toLocalInputFromDate(schedRange.end);
          dom.blobForm.defaultStart.dispatchEvent(new Event("change", { bubbles: true }));
        }
        dom.formStatus.textContent = "Ranges captured. Fill details and create.";
        clearSelectionCarets(".selection-caret.schedulable-range");
      }
    };

    dayColumns.forEach((column, columnIndex) => {
      const track = column.querySelector(".week-day-track");
      const onClick = (event) => {
        if (event.button !== 0) return;
        if (event.target.closest(".day-block")) return;
        if (state.selectionStep === null) return;
        const minutes = toMinutes(event.clientY, track);
        if (clickStart === null) {
          clickStart = minutes;
          activeColumnIndex = columnIndex;
          const endMin = Math.min(trackMinutes, minutes + minuteGranularity);
          const overlaySelector =
            state.selectionStep === "default"
              ? ".selection-overlay.default-range"
              : ".selection-overlay.schedulable-range";
          const caretSelector =
            state.selectionStep === "default"
              ? ".selection-caret.default-range"
              : ".selection-caret.schedulable-range";
          updateSelectionRange(columnIndex, minutes, columnIndex, endMin, overlaySelector);
          clearSelectionCarets(caretSelector);
          updateCaret(column.querySelector(caretSelector), minutes, hourHeight);
        } else {
          const sameDay = activeColumnIndex === columnIndex;
          const startMin = Math.min(clickStart, minutes);
          const endMin = sameDay
            ? Math.min(
                trackMinutes,
                Math.max(clickStart + minuteGranularity, minutes)
              )
            : Math.min(trackMinutes, minutes);
          const overlaySelector =
            state.selectionStep === "default"
              ? ".selection-overlay.default-range"
              : ".selection-overlay.schedulable-range";
          updateSelectionRange(activeColumnIndex, clickStart, columnIndex, endMin, overlaySelector);
          finalizeRange(activeColumnIndex, clickStart, columnIndex, endMin);
          clickStart = null;
          activeColumnIndex = null;
        }
      };

      const onMouseMove = (event) => {
        if (state.selectionStep === null) return;
        const minutes = toMinutes(event.clientY, track);
        const sameDay = activeColumnIndex === columnIndex;
        const endMin = sameDay
          ? Math.min(trackMinutes, Math.max(clickStart + minuteGranularity, minutes))
          : Math.min(trackMinutes, minutes);
        const overlaySelector =
          state.selectionStep === "default"
            ? ".selection-overlay.default-range"
            : ".selection-overlay.schedulable-range";
        const caretSelector =
          state.selectionStep === "default"
            ? ".selection-caret.default-range"
            : ".selection-caret.schedulable-range";
        if (clickStart === null) {
          clearSelectionCarets(caretSelector);
          updateCaret(column.querySelector(caretSelector), minutes, hourHeight);
        } else {
          updateSelectionRange(activeColumnIndex, clickStart, columnIndex, endMin, overlaySelector);
          clearSelectionCarets(caretSelector);
          updateCaret(dayColumns[activeColumnIndex].querySelector(caretSelector), clickStart, hourHeight);
        }
        state.selectionPointer = { x: event.clientX, y: event.clientY, columnIndex };
      };

      track.addEventListener("click", onClick);
      track.addEventListener("mousemove", onMouseMove);
    });

    if (state.selectionScrollHandler) {
      window.removeEventListener("scroll", state.selectionScrollHandler);
      window.removeEventListener("resize", state.selectionScrollHandler);
    }
    state.selectionScrollHandler = () => {
      if (state.selectionStep === null) return;
      if (!state.selectionPointer) return;
      const target = document.elementFromPoint(
        state.selectionPointer.x,
        state.selectionPointer.y
      );
      const columnEl = target ? target.closest(".week-day-column") : null;
      const columnIndex =
        (columnEl && dayColumns.indexOf(columnEl)) ??
        state.selectionPointer.columnIndex ??
        activeColumnIndex;
      if (columnIndex === null || columnIndex === undefined || columnIndex < 0) return;
      const track = dayColumns[columnIndex].querySelector(".week-day-track");
      const minutes = toMinutes(state.selectionPointer.y, track);
      const sameDay = activeColumnIndex === columnIndex;
      const endMin = sameDay
        ? Math.min(trackMinutes, Math.max(clickStart + minuteGranularity, minutes))
        : Math.min(trackMinutes, minutes);
      const overlaySelector =
        state.selectionStep === "default"
          ? ".selection-overlay.default-range"
          : ".selection-overlay.schedulable-range";
      const caretSelector =
        state.selectionStep === "default"
          ? ".selection-caret.default-range"
          : ".selection-caret.schedulable-range";
      if (clickStart === null) {
        clearSelectionCarets(caretSelector);
        updateCaret(dayColumns[columnIndex].querySelector(caretSelector), minutes, hourHeight);
      } else {
        updateSelectionRange(activeColumnIndex, clickStart, columnIndex, endMin, overlaySelector);
        clearSelectionCarets(caretSelector);
        updateCaret(dayColumns[activeColumnIndex].querySelector(caretSelector), clickStart, hourHeight);
      }
    };
    window.addEventListener("scroll", state.selectionScrollHandler, { passive: true });
    window.addEventListener("resize", state.selectionScrollHandler);
  }

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
        const start = toZonedDate(
          toDate(
          blob.realized_timerange?.start || blob.default_scheduled_timerange?.start
        ),
          appConfig.userTimeZone
        );
        const end = toZonedDate(
          toDate(
          blob.realized_timerange?.end || blob.default_scheduled_timerange?.end
        ),
          appConfig.userTimeZone
        );
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

  dom.views.month.innerHTML = `<div class="month-grid">${cards}</div>`;
  setDateLabel(formatMonthLabel(state.anchorDate));
}

function renderYear() {
  const year = state.anchorDate.getFullYear();
  const quarters = [0, 3, 6, 9].map((month) => new Date(year, month, 1));
  const cards = quarters
    .map((quarterStart, idx) => {
      const quarterEnd = new Date(year, quarterStart.getMonth() + 3, 1);
      const events = state.blobs.filter((blob) => {
        const start = toZonedDate(
          toDate(
          blob.realized_timerange?.start || blob.default_scheduled_timerange?.start
        ),
          appConfig.userTimeZone
        );
        const end = toZonedDate(
          toDate(
          blob.realized_timerange?.end || blob.default_scheduled_timerange?.end
        ),
          appConfig.userTimeZone
        );
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

  dom.views.year.innerHTML = `<div class="year-grid">${cards}</div>`;
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
  saveView(view);
  dom.tabs.forEach((tab) => tab.classList.toggle("active", tab.dataset.view === view));
  Object.entries(dom.views).forEach(([key, el]) => {
    el.classList.toggle("active", key === view);
  });
  if (dom.prevDayBtn && dom.nextDayBtn) {
    const labelMap = { day: "day", week: "week", month: "month", year: "year" };
    const label = labelMap[view] || "day";
    dom.prevDayBtn.title = `Previous ${label}`;
    dom.nextDayBtn.title = `Next ${label}`;
  }

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

function startInteractiveCreate() {
  state.selectionMode = true;
  state.selectionStep = "default";
  state.pendingDefaultRange = null;
  state.pendingSchedulableRange = null;
  dom.formStatus.textContent = "Click start/end for default range.";
  if (state.view !== "day" && state.view !== "week") {
    setActive("day");
  }
  renderAll();
  setActive(state.view);
}

export { renderAll, renderDay, renderMonth, renderWeek, renderYear, setActive, startInteractiveCreate };
