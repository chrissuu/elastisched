from __future__ import annotations

from backend.llm.base import BaseModelProvider, Message, ModelResponse
from backend.llm.tools import OpenAPIToolRegistry


class ToolCallingRuntime:
    def __init__(
        self,
        provider: BaseModelProvider,
        tool_registry: OpenAPIToolRegistry,
        max_steps: int = 6,
    ) -> None:
        self._provider = provider
        self._tool_registry = tool_registry
        self._max_steps = max_steps

    async def run(self, messages: list[Message]) -> tuple[ModelResponse, list[Message]]:
        conversation = list(messages)
        response = ModelResponse(text="", tool_calls=[])
        for _ in range(self._max_steps):
            response = await self._provider.generate(
                conversation, self._tool_registry.list_tools()
            )
            if response.tool_calls:
                conversation.append(
                    Message(role="assistant", tool_calls=response.tool_calls)
                )
                results = await self._tool_registry.execute_all(response.tool_calls)
                conversation.append(Message(role="tool", tool_results=results))
                continue
            conversation.append(
                Message(role="assistant", content=response.text or "")
            )
            return response, conversation
        return response, conversation
