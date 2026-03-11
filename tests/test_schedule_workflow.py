import os
import importlib
from datetime import datetime, timedelta, timezone
from zoneinfo import ZoneInfo

import pytest
import pytest_asyncio
from httpx import ASGITransport, AsyncClient

from .constants import AFTERNOON_START, WORK_TAG


@pytest_asyncio.fixture
async def api_client(tmp_path_factory):
    db_path = tmp_path_factory.mktemp("db") / "test.db"
    os.environ["DATABASE_URL"] = f"sqlite+aiosqlite:///{db_path}"
    os.environ["ELASTISCHED_PROJECT_TZ"] = "UTC"

    from backend import db as db_module
    from backend import main as main_module
    from backend import models as models_module

    importlib.reload(db_module)
    importlib.reload(models_module)
    importlib.reload(main_module)

    await db_module.init_db()
    transport = ASGITransport(app=main_module.app)
    return AsyncClient(transport=transport, base_url="http://test")


def _next_weekday_date(current: datetime, target_weekday: int) -> datetime.date:
    delta_days = (target_weekday - current.weekday()) % 7
    if delta_days == 0:
        delta_days = 7
    return (current + timedelta(days=delta_days)).date()


@pytest.mark.asyncio
@pytest.mark.skip(reason="Deprecated: non-primitive cost function behavior is handled by preference learning.")
async def test_frontend_to_scheduler_friday_afternoon_cost(api_client):
    user_tz = ZoneInfo("America/Los_Angeles")
    project_tz = timezone.utc

    now_local = datetime.now(timezone.utc).astimezone(user_tz)
    friday_date = _next_weekday_date(now_local, 4)
    thursday_date = friday_date - timedelta(days=1)

    schedulable_start_local = datetime(
        thursday_date.year,
        thursday_date.month,
        thursday_date.day,
        9,
        0,
        tzinfo=user_tz,
    )
    if schedulable_start_local <= now_local:
        friday_date = friday_date + timedelta(days=7)
        thursday_date = thursday_date + timedelta(days=7)
        schedulable_start_local = datetime(
            thursday_date.year,
            thursday_date.month,
            thursday_date.day,
            9,
            0,
            tzinfo=user_tz,
        )
    schedulable_end_local = datetime(
        friday_date.year,
        friday_date.month,
        friday_date.day,
        20,
        0,
        tzinfo=user_tz,
    )
    default_start_local = datetime(
        friday_date.year,
        friday_date.month,
        friday_date.day,
        18,
        30,
        tzinfo=user_tz,
    )
    default_end_local = datetime(
        friday_date.year,
        friday_date.month,
        friday_date.day,
        19,
        0,
        tzinfo=user_tz,
    )

    def to_project_iso(value: datetime) -> str:
        return value.astimezone(project_tz).isoformat()

    recurrence_payload = {
        "type": "single",
        "payload": {
            "blob": {
                "name": "Friday Focus",
                "description": "Avoid late Friday work",
                "tz": "UTC",
                "default_scheduled_timerange": {
                    "start": to_project_iso(default_start_local),
                    "end": to_project_iso(default_end_local),
                },
                "schedulable_timerange": {
                    "start": to_project_iso(schedulable_start_local),
                    "end": to_project_iso(schedulable_end_local),
                },
                "policy": {},
                "dependencies": [],
                "tags": [WORK_TAG],
            }
        },
    }

    async with api_client as client:
        create_resp = await client.post("/recurrences", json=recurrence_payload)
        assert create_resp.status_code == 201
        recurrence_id = create_resp.json()["id"]

        schedule_payload = {
            "granularity_minutes": 15,
            "lookahead_seconds": 21 * 24 * 60 * 60,
            "user_timezone": user_tz.key,
        }
        schedule_resp = await client.post("/schedule", json=schedule_payload)
        assert schedule_resp.status_code == 200
        schedule_data = schedule_resp.json()

    occurrences = schedule_data.get("occurrences") or []
    occurrence = next(
        (item for item in occurrences if item.get("recurrence_id") == recurrence_id),
        None,
    )
    assert occurrence is not None
    realized = occurrence.get("realized_timerange")
    assert realized is not None

    realized_start = datetime.fromisoformat(realized["start"])
    realized_start_local = realized_start.astimezone(user_tz)

    assert schedulable_start_local <= realized_start_local <= schedulable_end_local
    if realized_start_local.weekday() == 4:
        assert realized_start_local.hour < AFTERNOON_START.value


@pytest.mark.asyncio
@pytest.mark.skip(reason="Deprecated: non-primitive cost preferences handled by learner.")
async def test_single_occurrence_scheduled_same_day_before_afternoon(
    api_client, monkeypatch
):
    user_tz = ZoneInfo("America/New_York")
    project_tz = timezone.utc

    fixed_now = datetime(2026, 1, 1, 12, 0, tzinfo=timezone.utc)

    import backend.schedule_router as schedule_router

    class FrozenDateTime(datetime):
        @classmethod
        def now(cls, tz=None):
            if tz is None:
                return fixed_now.replace(tzinfo=None)
            return fixed_now.astimezone(tz)

    monkeypatch.setattr(schedule_router, "datetime", FrozenDateTime)

    target_date = datetime(2026, 1, 2, tzinfo=user_tz).date()
    schedulable_start_local = datetime(
        target_date.year,
        target_date.month,
        target_date.day,
        1,
        25,
        tzinfo=user_tz,
    )
    schedulable_end_local = datetime(
        target_date.year,
        target_date.month,
        target_date.day,
        23,
        25,
        tzinfo=user_tz,
    )
    default_start_local = datetime(
        target_date.year,
        target_date.month,
        target_date.day,
        21,
        10,
        tzinfo=user_tz,
    )
    default_end_local = datetime(
        target_date.year,
        target_date.month,
        target_date.day,
        22,
        55,
        tzinfo=user_tz,
    )

    def to_project_iso(value: datetime) -> str:
        return value.astimezone(project_tz).isoformat()

    recurrence_payload = {
        "type": "single",
        "payload": {
            "blob": {
                "name": "Friday Night Work",
                "description": "Should move earlier",
                "tz": "UTC",
                "default_scheduled_timerange": {
                    "start": to_project_iso(default_start_local),
                    "end": to_project_iso(default_end_local),
                },
                "schedulable_timerange": {
                    "start": to_project_iso(schedulable_start_local),
                    "end": to_project_iso(schedulable_end_local),
                },
                "policy": {},
                "dependencies": [],
                "tags": [WORK_TAG],
            }
        },
    }

    async with api_client as client:
        create_resp = await client.post("/recurrences", json=recurrence_payload)
        assert create_resp.status_code == 201
        recurrence_id = create_resp.json()["id"]

        schedule_payload = {
            "granularity_minutes": 15,
            "lookahead_seconds": 1209600,
            "user_timezone": user_tz.key,
        }
        schedule_resp = await client.post("/schedule", json=schedule_payload)
        assert schedule_resp.status_code == 200
        schedule_data = schedule_resp.json()

    occurrences = schedule_data.get("occurrences") or []
    occurrence = next(
        (item for item in occurrences if item.get("recurrence_id") == recurrence_id),
        None,
    )
    assert occurrence is not None
    realized = occurrence.get("realized_timerange")
    assert realized is not None

    realized_start = datetime.fromisoformat(realized["start"])
    realized_start_local = realized_start.astimezone(user_tz)

    assert realized_start_local.date() == target_date
    assert realized_start_local.hour < AFTERNOON_START.value


@pytest.mark.asyncio
async def test_schedule_includes_overlapping_schedulable_window_started_before_now(
    api_client, monkeypatch
):
    fixed_now = datetime(2026, 2, 25, 16, 20, tzinfo=timezone.utc)
    user_tz = ZoneInfo("America/New_York")

    import backend.schedule_router as schedule_router

    class FrozenDateTime(datetime):
        @classmethod
        def now(cls, tz=None):
            if tz is None:
                return fixed_now.replace(tzinfo=None)
            return fixed_now.astimezone(tz)

    monkeypatch.setattr(schedule_router, "datetime", FrozenDateTime)

    schedulable_start = fixed_now - timedelta(minutes=30)
    schedulable_end = fixed_now + timedelta(days=2)
    default_start = fixed_now + timedelta(hours=2)
    default_end = default_start + timedelta(hours=1)

    recurrence_payload = {
        "type": "single",
        "payload": {
            "blob": {
                "name": "Window crossing now",
                "description": "Should still be scheduled",
                "tz": "UTC",
                "default_scheduled_timerange": {
                    "start": default_start.isoformat(),
                    "end": default_end.isoformat(),
                },
                "schedulable_timerange": {
                    "start": schedulable_start.isoformat(),
                    "end": schedulable_end.isoformat(),
                },
                "policy": {"is_overlappable": False},
                "dependencies": [],
                "tags": [],
            }
        },
    }

    async with api_client as client:
        create_resp = await client.post("/recurrences", json=recurrence_payload)
        assert create_resp.status_code == 201
        recurrence_id = create_resp.json()["id"]
        expected_occurrence_id = f"{recurrence_id}:{schedulable_start.isoformat()}"

        schedule_payload = {
            "granularity_minutes": 5,
            "lookahead_seconds": 3 * 24 * 60 * 60,
            "user_timezone": user_tz.key,
        }
        schedule_resp = await client.post("/schedule", json=schedule_payload)
        assert schedule_resp.status_code == 200
        schedule_data = schedule_resp.json()

    occurrences = schedule_data.get("occurrences") or []
    occurrence = next(
        (item for item in occurrences if item.get("id") == expected_occurrence_id),
        None,
    )
    assert occurrence is not None
    assert occurrence.get("realized_timerange") is not None


@pytest.mark.asyncio
async def test_occurrences_preserve_show_on_tasks_page_policy(api_client):
    schedulable_start = datetime(2026, 3, 3, 14, 0, tzinfo=timezone.utc)
    schedulable_end = datetime(2026, 3, 3, 16, 0, tzinfo=timezone.utc)
    default_start = datetime(2026, 3, 3, 14, 30, tzinfo=timezone.utc)
    default_end = datetime(2026, 3, 3, 15, 0, tzinfo=timezone.utc)

    recurrence_payload = {
        "type": "single",
        "payload": {
            "blob": {
                "name": "Quiet task",
                "description": "Should stay off the Tasks page",
                "tz": "UTC",
                "default_scheduled_timerange": {
                    "start": default_start.isoformat(),
                    "end": default_end.isoformat(),
                },
                "schedulable_timerange": {
                    "start": schedulable_start.isoformat(),
                    "end": schedulable_end.isoformat(),
                },
                "policy": {"show_on_tasks_page": False},
                "dependencies": [],
                "tags": [],
            }
        },
    }

    async with api_client as client:
        create_resp = await client.post("/recurrences", json=recurrence_payload)
        assert create_resp.status_code == 201

        occurrence_resp = await client.get(
            "/occurrences",
            params={
                "start": datetime(2026, 3, 1, tzinfo=timezone.utc).isoformat(),
                "end": datetime(2026, 3, 10, tzinfo=timezone.utc).isoformat(),
            },
        )
        assert occurrence_resp.status_code == 200

    occurrences = occurrence_resp.json()
    occurrence = next((item for item in occurrences if item.get("name") == "Quiet task"), None)
    assert occurrence is not None
    assert occurrence["policy"]["show_on_tasks_page"] is False


@pytest.mark.asyncio
async def test_occurrence_timing_override_applies_and_finished_override_skips_schedule(
    api_client, monkeypatch
):
    fixed_now = datetime(2026, 3, 9, 12, 0, tzinfo=timezone.utc)

    import backend.schedule_router as schedule_router

    class FrozenDateTime(datetime):
        @classmethod
        def now(cls, tz=None):
            if tz is None:
                return fixed_now.replace(tzinfo=None)
            return fixed_now.astimezone(tz)

    monkeypatch.setattr(schedule_router, "datetime", FrozenDateTime)

    base_default_start = datetime(2026, 3, 9, 18, 0, tzinfo=timezone.utc)
    base_default_end = datetime(2026, 3, 9, 19, 0, tzinfo=timezone.utc)
    base_sched_start = datetime(2026, 3, 9, 17, 0, tzinfo=timezone.utc)
    base_sched_end = datetime(2026, 3, 9, 21, 0, tzinfo=timezone.utc)
    occurrence_key = base_sched_start.isoformat()

    moved_default_start = base_default_start + timedelta(hours=1)
    moved_default_end = base_default_end + timedelta(hours=1)
    moved_sched_start = base_sched_start + timedelta(hours=1)
    moved_sched_end = base_sched_end + timedelta(hours=1)

    def parse_iso(value: str) -> datetime:
        return datetime.fromisoformat(value.replace("Z", "+00:00"))

    weekly_payload = {
        "recurrence_name": "Weekly override case",
        "interval": 1,
        "blobs_of_week": [
            {
                "name": "Weekly override case",
                "description": "Verify per-occurrence timing override",
                "tz": "UTC",
                "default_scheduled_timerange": {
                    "start": base_default_start.isoformat(),
                    "end": base_default_end.isoformat(),
                },
                "schedulable_timerange": {
                    "start": base_sched_start.isoformat(),
                    "end": base_sched_end.isoformat(),
                },
                "policy": {},
                "dependencies": [],
                "tags": [],
            }
        ],
    }

    async with api_client as client:
        create_resp = await client.post(
            "/recurrences",
            json={"type": "weekly", "payload": weekly_payload},
        )
        assert create_resp.status_code == 201
        recurrence_id = create_resp.json()["id"]

        updated_payload = {
            **weekly_payload,
            "occurrence_overrides": {
                occurrence_key: {
                    "default_scheduled_timerange": {
                        "start": moved_default_start.isoformat(),
                        "end": moved_default_end.isoformat(),
                    },
                    "schedulable_timerange": {
                        "start": moved_sched_start.isoformat(),
                        "end": moved_sched_end.isoformat(),
                    },
                    "finished_at": (moved_default_end + timedelta(minutes=10)).isoformat(),
                }
            },
        }
        update_resp = await client.put(
            f"/recurrences/{recurrence_id}",
            json={"type": "weekly", "payload": updated_payload},
        )
        assert update_resp.status_code == 200

        occurrences_resp = await client.get(
            "/occurrences",
            params={
                "start": fixed_now.isoformat(),
                "end": (fixed_now + timedelta(days=2)).isoformat(),
            },
        )
        assert occurrences_resp.status_code == 200
        occurrences = occurrences_resp.json()
        updated_occurrence = next(
            (item for item in occurrences if item.get("recurrence_id") == recurrence_id),
            None,
        )
        assert updated_occurrence is not None
        assert parse_iso(
            updated_occurrence["default_scheduled_timerange"]["start"]
        ) == moved_default_start
        assert parse_iso(
            updated_occurrence["default_scheduled_timerange"]["end"]
        ) == moved_default_end
        assert parse_iso(
            updated_occurrence["schedulable_timerange"]["start"]
        ) == moved_sched_start
        assert parse_iso(
            updated_occurrence["schedulable_timerange"]["end"]
        ) == moved_sched_end

        schedule_resp = await client.post(
            "/schedule",
            json={
                "granularity_minutes": 5,
                "lookahead_seconds": 3 * 24 * 60 * 60,
                "user_timezone": "UTC",
            },
        )
        assert schedule_resp.status_code == 200
        scheduled_occurrences = schedule_resp.json().get("occurrences") or []

    scheduled_occurrence = next(
        (item for item in scheduled_occurrences if item.get("recurrence_id") == recurrence_id),
        None,
    )
    assert scheduled_occurrence is not None
    assert scheduled_occurrence.get("realized_timerange") is None
