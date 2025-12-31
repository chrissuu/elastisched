const data = {
  day: {
    dateLabel: "Tuesday, May 21",
    hours: ["8 AM", "9 AM", "10 AM", "11 AM", "12 PM", "1 PM", "2 PM", "3 PM", "4 PM", "5 PM"],
    slots: [
      [
        { title: "Daily Review", time: "8:00 - 8:20", type: "admin" },
        { title: "Inbox Zero", time: "8:30 - 8:45", type: "admin" },
      ],
      [{ title: "Sprint Sync", time: "9:00 - 9:40", type: "focus" }],
      [
        { title: "Roadmap Deep Dive", time: "10:00 - 11:15", type: "deep" },
        { title: "Design QA", time: "10:30 - 11:00", type: "focus" },
        { title: "Quick Reply", time: "10:45 - 11:00", type: "admin" },
        { title: "Follow-ups", time: "11:00 - 11:15", type: "admin" },
      ],
      [],
      [{ title: "Lunch Walk", time: "12:10 - 12:50", type: "focus" }],
      [
        { title: "Prototype Build", time: "1:00 - 2:20", type: "deep" },
        { title: "Customer Call", time: "1:30 - 1:50", type: "focus" },
      ],
      [{ title: "Spec Review", time: "2:30 - 3:10", type: "focus" }],
      [],
      [{ title: "Ops Cleanup", time: "4:00 - 4:30", type: "admin" }],
      [{ title: "Wrap Notes", time: "5:10 - 5:25", type: "admin" }],
    ],
  },
  week: {
    label: "Week of May 20",
    days: [
      { name: "Mon", events: ["Kickoff", "Focus Sprint", "Draft Plan"] },
      { name: "Tue", events: ["Review", "Prototype", "Ops"] },
      { name: "Wed", events: ["Research", "Workshop", "1:1s", "3 more"] },
      { name: "Thu", events: ["Launch Prep", "QA", "Stakeholder"] },
      { name: "Fri", events: ["Retro", "Roadmap", "Admin"] },
      { name: "Sat", events: ["Personal", "Family"] },
      { name: "Sun", events: ["Reset", "Plan"] },
    ],
  },
  month: {
    label: "May 2024",
    weeks: [
      { title: "Week 1", summary: "8 meetings, 3 focus blocks" },
      { title: "Week 2", summary: "12 meetings, 5 focus blocks" },
      { title: "Week 3", summary: "9 meetings, 4 focus blocks" },
      { title: "Week 4", summary: "7 meetings, 6 focus blocks" },
    ],
  },
  year: {
    label: "2024",
    quarters: [
      { title: "Q1", summary: "Launch planning, 42 sessions" },
      { title: "Q2", summary: "Build + beta, 38 sessions" },
      { title: "Q3", summary: "Growth experiments, 31 sessions" },
      { title: "Q4", summary: "Wrap + retros, 27 sessions" },
    ],
  },
};

const dateLabel = document.getElementById("dateLabel");
const tabs = document.querySelectorAll(".tab");
const views = {
  day: document.getElementById("viewDay"),
  week: document.getElementById("viewWeek"),
  month: document.getElementById("viewMonth"),
  year: document.getElementById("viewYear"),
};

function renderDay() {
  const { hours, slots } = data.day;
  const hoursHtml = hours.map((hour) => `<div class="hour">${hour}</div>`).join("");
  const slotHtml = slots
    .map((events) => {
      const visible = events.slice(0, 2);
      const remaining = events.length - visible.length;
      const eventsHtml = visible
        .map(
          (event) => `
          <div class="event ${event.type}">
            <span>${event.title}</span>
            <span class="event-time">${event.time}</span>
          </div>
        `
        )
        .join("");
      const moreHtml =
        remaining > 0 ? `<div class="more">+${remaining} more</div>` : "";
      return `<div class="slot">${eventsHtml}${moreHtml}</div>`;
    })
    .join("");

  views.day.innerHTML = `
    <div class="day-grid">
      <div class="hours">${hoursHtml}</div>
      <div class="slots">${slotHtml}</div>
    </div>
  `;
}

function renderWeek() {
  const cards = data.week.days
    .map((day) => {
      const events = day.events
        .slice(0, 3)
        .map(
          (event) => `<div class="card-event"><span>${event}</span><span>•</span></div>`
        )
        .join("");
      const overflow = day.events.length > 3 ? "More packed than usual" : "Balanced day";
      return `
        <div class="card">
          <div class="card-title">${day.name}</div>
          ${events}
          <div class="card-summary">${overflow}</div>
        </div>
      `;
    })
    .join("");
  views.week.innerHTML = `<div class="week-grid">${cards}</div>`;
}

function renderMonth() {
  const cards = data.month.weeks
    .map(
      (week) => `
        <div class="card">
          <div class="card-title">${week.title}</div>
          <div class="card-summary">${week.summary}</div>
          <div class="card-event"><span>Top focus days</span><span>Tue, Thu</span></div>
          <div class="card-event"><span>Compression</span><span>Low</span></div>
        </div>
      `
    )
    .join("");
  views.month.innerHTML = `<div class="month-grid">${cards}</div>`;
}

function renderYear() {
  const cards = data.year.quarters
    .map(
      (quarter) => `
        <div class="card">
          <div class="card-title">${quarter.title}</div>
          <div class="card-summary">${quarter.summary}</div>
          <div class="card-event"><span>Peak week</span><span>Week 18</span></div>
          <div class="card-event"><span>Flow score</span><span>0.72</span></div>
        </div>
      `
    )
    .join("");
  views.year.innerHTML = `<div class="year-grid">${cards}</div>`;
}

function setActive(view) {
  tabs.forEach((tab) => tab.classList.toggle("active", tab.dataset.view === view));
  Object.entries(views).forEach(([key, el]) => {
    el.classList.toggle("active", key === view);
  });

  if (view === "day") {
    dateLabel.textContent = data.day.dateLabel;
  } else if (view === "week") {
    dateLabel.textContent = data.week.label;
  } else if (view === "month") {
    dateLabel.textContent = data.month.label;
  } else {
    dateLabel.textContent = data.year.label;
  }
}

tabs.forEach((tab) => {
  tab.addEventListener("click", () => setActive(tab.dataset.view));
});

renderDay();
renderWeek();
renderMonth();
renderYear();
setActive("day");
