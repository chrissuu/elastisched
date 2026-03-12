# Frontend

`frontend/` is the browser UI for Elastisched (vanilla HTML/CSS/JS).

## Structure
- [`frontend/index.html`](index.html): app shell and modal/layout markup.
- [`frontend/css/styles.css`](css/styles.css): visual theme and layout styles.
- [`frontend/js/app.js`](js/app.js): app bootstrap + workspace behavior.
- [`frontend/js/auth.js`](js/auth.js): authenticated session bootstrap + CSRF-aware fetch wrapper.
- [`frontend/js/api.js`](js/api.js): API client calls (`/occurrences`, `/schedule`, `/recurrences`, `/llm`, `/integrations`).
- [`frontend/js/render.js`](js/render.js): calendar/task rendering logic.
- [`frontend/js/forms.js`](js/forms.js): create/edit/settings form handling.
- [`frontend/js/integrations.js`](js/integrations.js): integration UI flows.

## Serving
- From backend static mount: `http://localhost:8000/ui` (requires a valid session from `/`).
- Through nginx in Docker: `http://localhost:8080/ui`.
