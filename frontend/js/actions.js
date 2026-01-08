import { createRecurrence, deleteRecurrence, getRecurrence, updateRecurrence } from "./api.js";
import { pushHistoryAction } from "./history.js";

function refreshCalendar() {
  window.dispatchEvent(new CustomEvent("elastisched:refresh"));
}

async function deleteRecurrenceWithUndo(recurrenceId) {
  if (!recurrenceId) return;
  const previous = await getRecurrence(recurrenceId);
  await deleteRecurrence(recurrenceId);
  pushHistoryAction({
    type: "delete-recurrence",
    data: {
      recurrenceId,
      recurrenceType: previous.type,
      payload: previous.payload,
      restoredId: null,
    },
  });
  refreshCalendar();
}

async function deleteOccurrenceWithUndo(blob) {
  if (!blob?.recurrence_id) return;
  const occurrenceStart = blob.schedulable_timerange?.start;
  if (!occurrenceStart) return;
  const previous = await getRecurrence(blob.recurrence_id);
  const payload = previous.payload || {};
  const existing = Array.isArray(payload.exclusions) ? payload.exclusions : [];
  const nextExclusions = Array.from(new Set([...existing, occurrenceStart]));
  const nextPayload = { ...payload, exclusions: nextExclusions };
  const recurrenceType = previous.type || blob.recurrence_type || "single";
  await updateRecurrence(blob.recurrence_id, recurrenceType, nextPayload);
  pushHistoryAction({
    type: "update-recurrence",
    data: {
      recurrenceId: blob.recurrence_id,
      recurrenceType,
      beforePayload: payload,
      afterPayload: nextPayload,
    },
  });
  refreshCalendar();
}

export { deleteOccurrenceWithUndo, deleteRecurrenceWithUndo };
