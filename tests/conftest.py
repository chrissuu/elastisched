from __future__ import annotations

import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BACKEND = ROOT / "backend"
SRC = ROOT / "src"

for path in (BACKEND, SRC):
    if path.exists():
        sys.path.insert(0, str(path))
