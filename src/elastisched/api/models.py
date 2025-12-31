from datetime import datetime

from sqlalchemy import DateTime, JSON, String
from sqlalchemy.orm import Mapped, mapped_column

from elastisched.api.db import Base


class BlobModel(Base):
    __tablename__ = "blobs"

    id: Mapped[str] = mapped_column(String(36), primary_key=True)
    name: Mapped[str] = mapped_column(String(200))
    description: Mapped[str | None] = mapped_column(String(2000), nullable=True)
    tz: Mapped[str] = mapped_column(String(64), default="UTC")
    default_scheduled_start: Mapped[datetime] = mapped_column(DateTime(timezone=True))
    default_scheduled_end: Mapped[datetime] = mapped_column(DateTime(timezone=True))
    schedulable_start: Mapped[datetime] = mapped_column(DateTime(timezone=True))
    schedulable_end: Mapped[datetime] = mapped_column(DateTime(timezone=True))
    realized_start: Mapped[datetime | None] = mapped_column(DateTime(timezone=True), nullable=True)
    realized_end: Mapped[datetime | None] = mapped_column(DateTime(timezone=True), nullable=True)
    policy: Mapped[dict] = mapped_column(JSON, default=dict)
    dependencies: Mapped[list] = mapped_column(JSON, default=list)
    tags: Mapped[list] = mapped_column(JSON, default=list)
