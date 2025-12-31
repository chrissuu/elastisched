from datetime import datetime

from pydantic import BaseModel, ConfigDict, Field


class TimeRangeSchema(BaseModel):
    start: datetime
    end: datetime


class BlobBase(BaseModel):
    name: str = Field(min_length=1, max_length=200)
    description: str | None = Field(default=None, max_length=2000)
    default_scheduled_timerange: TimeRangeSchema
    schedulable_timerange: TimeRangeSchema
    realized_timerange: TimeRangeSchema | None = None
    tz: str = "UTC"
    policy: dict = Field(default_factory=dict)
    dependencies: list[str] = Field(default_factory=list)
    tags: list[str] = Field(default_factory=list)


class BlobCreate(BlobBase):
    pass


class BlobUpdate(BaseModel):
    name: str | None = Field(default=None, min_length=1, max_length=200)
    description: str | None = Field(default=None, max_length=2000)
    default_scheduled_timerange: TimeRangeSchema | None = None
    schedulable_timerange: TimeRangeSchema | None = None
    realized_timerange: TimeRangeSchema | None = None
    tz: str | None = None
    policy: dict | None = None
    dependencies: list[str] | None = None
    tags: list[str] | None = None


class BlobRead(BlobBase):
    id: str

    model_config = ConfigDict(from_attributes=True)
