import os
import importlib
from datetime import datetime, timedelta, timezone

import pytest
from httpx import ASGITransport, AsyncClient


@pytest.fixture
def api_client(tmp_path_factory):
    db_path = tmp_path_factory.mktemp("db") / "test.db"
    os.environ["DATABASE_URL"] = f"sqlite+aiosqlite:///{db_path}"

    from elastisched.api import db as db_module
    from elastisched.api import main as main_module

    importlib.reload(db_module)
    importlib.reload(main_module)

    transport = ASGITransport(app=main_module.app, lifespan="on")
    return AsyncClient(transport=transport, base_url="http://test")


@pytest.mark.asyncio
async def test_create_and_get_blob(api_client):
    start = datetime(2024, 1, 1, 9, 0, tzinfo=timezone.utc)
    end = start + timedelta(hours=2)
    payload = {
        "name": "Morning Focus",
        "description": "Deep work block",
        "tz": "UTC",
        "default_scheduled_timerange": {"start": start.isoformat(), "end": end.isoformat()},
        "schedulable_timerange": {
            "start": (start - timedelta(hours=1)).isoformat(),
            "end": (end + timedelta(hours=1)).isoformat(),
        },
        "policy": {"kind": "fixed"},
        "dependencies": [],
        "tags": ["focus"],
    }

    async with api_client as client:
        create_resp = await client.post("/blobs", json=payload)
        assert create_resp.status_code == 201
        created = create_resp.json()

        get_resp = await client.get(f"/blobs/{created['id']}")
        assert get_resp.status_code == 200
        fetched = get_resp.json()

    assert fetched["name"] == payload["name"]
    assert fetched["default_scheduled_timerange"]["start"] == payload["default_scheduled_timerange"]["start"]


@pytest.mark.asyncio
async def test_update_blob(api_client):
    start = datetime(2024, 2, 1, 9, 0, tzinfo=timezone.utc)
    end = start + timedelta(hours=1)
    payload = {
        "name": "Standup",
        "description": "Daily sync",
        "tz": "UTC",
        "default_scheduled_timerange": {"start": start.isoformat(), "end": end.isoformat()},
        "schedulable_timerange": {
            "start": (start - timedelta(minutes=30)).isoformat(),
            "end": (end + timedelta(minutes=30)).isoformat(),
        },
        "policy": {},
        "dependencies": [],
        "tags": [],
    }

    async with api_client as client:
        create_resp = await client.post("/blobs", json=payload)
        blob_id = create_resp.json()["id"]

        update_resp = await client.put(
            f"/blobs/{blob_id}",
            json={"name": "Standup Updated", "tags": ["team"]},
        )
        assert update_resp.status_code == 200
        updated = update_resp.json()

    assert updated["name"] == "Standup Updated"
    assert updated["tags"] == ["team"]


@pytest.mark.asyncio
async def test_list_blobs_with_overlap_filter(api_client):
    base = datetime(2024, 3, 1, 9, 0, tzinfo=timezone.utc)
    payloads = [
        {
            "name": "Morning",
            "description": None,
            "tz": "UTC",
            "default_scheduled_timerange": {
                "start": base.isoformat(),
                "end": (base + timedelta(hours=1)).isoformat(),
            },
            "schedulable_timerange": {
                "start": (base - timedelta(hours=1)).isoformat(),
                "end": (base + timedelta(hours=2)).isoformat(),
            },
            "policy": {},
            "dependencies": [],
            "tags": [],
        },
        {
            "name": "Afternoon",
            "description": None,
            "tz": "UTC",
            "default_scheduled_timerange": {
                "start": (base + timedelta(hours=6)).isoformat(),
                "end": (base + timedelta(hours=7)).isoformat(),
            },
            "schedulable_timerange": {
                "start": (base + timedelta(hours=5)).isoformat(),
                "end": (base + timedelta(hours=8)).isoformat(),
            },
            "policy": {},
            "dependencies": [],
            "tags": [],
        },
    ]

    async with api_client as client:
        for payload in payloads:
            resp = await client.post("/blobs", json=payload)
            assert resp.status_code == 201

        overlap_start = (base + timedelta(minutes=30)).isoformat()
        overlap_end = (base + timedelta(hours=1, minutes=30)).isoformat()
        list_resp = await client.get(
            "/blobs",
            params={"overlaps_start": overlap_start, "overlaps_end": overlap_end},
        )
        assert list_resp.status_code == 200
        listed = list_resp.json()

    assert len(listed) == 1
    assert listed[0]["name"] == "Morning"
