FROM python:3.11-slim

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    PIP_NO_CACHE_DIR=1 \
    PYTHONPATH=/app/backend

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /app/data

COPY README.md requirements.txt pyproject.toml /app/
COPY src /app/src
COPY backend /app/backend
COPY frontend /app/frontend

RUN pip install --upgrade pip \
    && pip install -r requirements.txt \
    && pip install .

EXPOSE 8000

CMD ["uvicorn", "elastisched_api.main:app", "--host", "0.0.0.0", "--port", "8000"]
