from datetime import datetime

from pydantic import BaseModel, ConfigDict, Field


class UserProfileRead(BaseModel):
    id: str
    email: str
    display_name: str | None = None
    created_at: datetime

    model_config = ConfigDict(from_attributes=True)


class UserProfileUpdate(BaseModel):
    display_name: str | None = Field(default=None, min_length=1, max_length=120)


class RegisterRequest(BaseModel):
    email: str = Field(min_length=3, max_length=320)
    password: str = Field(min_length=8, max_length=256)
    display_name: str | None = Field(default=None, min_length=1, max_length=120)


class LoginRequest(BaseModel):
    email: str = Field(min_length=3, max_length=320)
    password: str = Field(min_length=1, max_length=256)


class SessionRead(BaseModel):
    user: UserProfileRead
    csrf_token: str
    expires_at: datetime
