from __future__ import annotations

import asyncio
from dataclasses import dataclass
from typing import Any
from urllib.parse import quote

import httpx

from elastisched_api.llm.base import ToolCall, ToolResult, ToolSpec


@dataclass(frozen=True)
class OpenAPIOperation:
    name: str
    method: str
    path: str
    parameters_schema: dict[str, Any]
    request_body_required: bool


def _resolve_schema(
    schema: dict[str, Any], components: dict[str, Any], seen: set[str] | None = None
) -> dict[str, Any]:
    if not schema:
        return {}
    ref = schema.get("$ref")
    if ref:
        seen = seen or set()
        if ref in seen:
            return schema
        seen.add(ref)
        if ref.startswith("#/"):
            node: Any = components
            for part in ref.lstrip("#/").split("/"):
                if not isinstance(node, dict):
                    return schema
                node = node.get(part)
                if node is None:
                    return schema
            if isinstance(node, dict):
                return _resolve_schema(node, components, seen)
        return schema

    if "properties" in schema and isinstance(schema["properties"], dict):
        resolved = {}
        for key, value in schema["properties"].items():
            if isinstance(value, dict):
                resolved[key] = _resolve_schema(value, components, seen)
            else:
                resolved[key] = value
        schema = dict(schema)
        schema["properties"] = resolved
    if "items" in schema and isinstance(schema["items"], dict):
        schema = dict(schema)
        schema["items"] = _resolve_schema(schema["items"], components, seen)
    return schema


def _schema_for_parameters(
    parameters: list[dict[str, Any]] | None, components: dict[str, Any]
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    path_props: dict[str, Any] = {}
    query_props: dict[str, Any] = {}
    headers_props: dict[str, Any] = {}
    path_required: list[str] = []
    query_required: list[str] = []
    headers_required: list[str] = []
    for param in parameters or []:
        location = param.get("in")
        name = param.get("name")
        if not location or not name:
            continue
        schema = param.get("schema") or {}
        if isinstance(schema, dict):
            schema = _resolve_schema(schema, components)
        else:
            schema = {}
        if location == "path":
            path_props[name] = schema
            if param.get("required"):
                path_required.append(name)
        elif location == "query":
            query_props[name] = schema
            if param.get("required"):
                query_required.append(name)
        elif location == "header":
            headers_props[name] = schema
            if param.get("required"):
                headers_required.append(name)

    def _group_schema(properties: dict[str, Any], required: list[str]) -> dict[str, Any]:
        if not properties:
            return {}
        group: dict[str, Any] = {"type": "object", "properties": properties}
        if required:
            group["required"] = required
        return group

    return (
        _group_schema(path_props, path_required),
        _group_schema(query_props, query_required),
        _group_schema(headers_props, headers_required),
    )


class OpenAPIToolRegistry:
    def __init__(
        self,
        base_url: str,
        tools: list[ToolSpec],
        operations: dict[str, OpenAPIOperation],
        client: httpx.AsyncClient | None = None,
    ) -> None:
        self._base_url = base_url.rstrip("/")
        self._tools = tools
        self._operations = operations
        self._client = client or httpx.AsyncClient(timeout=20.0)
        self._owns_client = client is None

    @classmethod
    def from_openapi(cls, openapi: dict[str, Any], base_url: str) -> "OpenAPIToolRegistry":
        components = openapi.get("components") or {}
        tools: list[ToolSpec] = []
        operations: dict[str, OpenAPIOperation] = {}
        for path, methods in (openapi.get("paths") or {}).items():
            if not isinstance(methods, dict):
                continue
            for method, operation in methods.items():
                if method.lower() not in {"get", "post", "put", "patch", "delete"}:
                    continue
                if not isinstance(operation, dict):
                    continue
                tags = operation.get("tags") or []
                if "llm" in tags:
                    continue
                name = operation.get("operationId")
                if not name:
                    continue
                summary = operation.get("summary") or operation.get("description") or ""
                path_schema, query_schema, headers_schema = _schema_for_parameters(
                    operation.get("parameters") or [], components
                )
                request_body = operation.get("requestBody") or {}
                body_schema = {}
                body_required = bool(request_body.get("required"))
                if request_body:
                    content = request_body.get("content") or {}
                    json_body = content.get("application/json") or {}
                    schema = json_body.get("schema") or {}
                    if isinstance(schema, dict):
                        body_schema = _resolve_schema(schema, components)
                parameters: dict[str, Any] = {"type": "object", "properties": {}}
                required_fields: list[str] = []
                if path_schema:
                    parameters["properties"]["path"] = path_schema
                if query_schema:
                    parameters["properties"]["query"] = query_schema
                if headers_schema:
                    parameters["properties"]["headers"] = headers_schema
                if body_schema:
                    parameters["properties"]["body"] = body_schema
                    if body_required:
                        required_fields.append("body")
                if required_fields:
                    parameters["required"] = required_fields
                tools.append(
                    ToolSpec(
                        name=name,
                        description=summary,
                        parameters=parameters,
                    )
                )
                operations[name] = OpenAPIOperation(
                    name=name,
                    method=method,
                    path=path,
                    parameters_schema=parameters,
                    request_body_required=body_required,
                )
        return cls(base_url=base_url, tools=tools, operations=operations)

    def list_tools(self) -> list[ToolSpec]:
        return list(self._tools)

    async def execute(self, call: ToolCall) -> ToolResult:
        operation = self._operations.get(call.name)
        if not operation:
            return ToolResult(
                id=call.id,
                name=call.name,
                output={"error": "Unknown tool", "tool": call.name},
            )
        args = call.arguments or {}
        path_args = args.get("path") or {}
        query_args = args.get("query") or {}
        headers_args = args.get("headers") or {}
        body = args.get("body")

        url_path = operation.path
        for key, value in path_args.items():
            url_path = url_path.replace(f"{{{key}}}", quote(str(value), safe=""))

        url = f"{self._base_url}{url_path}"
        response = await self._client.request(
            operation.method,
            url,
            params=query_args or None,
            headers=headers_args or None,
            json=body if body is not None else None,
        )
        try:
            payload = response.json()
        except ValueError:
            payload = response.text
        if response.status_code >= 400:
            payload = {
                "error": "Tool request failed",
                "status": response.status_code,
                "body": payload,
            }
        return ToolResult(id=call.id, name=call.name, output=payload)

    async def execute_all(self, calls: list[ToolCall]) -> list[ToolResult]:
        if not calls:
            return []
        return list(await asyncio.gather(*(self.execute(call) for call in calls)))

    async def aclose(self) -> None:
        if self._owns_client:
            await self._client.aclose()
