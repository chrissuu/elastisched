from fastapi import APIRouter, HTTPException, Request, status

from elastisched_api.config import get_gemini_api_key, get_gemini_model
from elastisched_api.llm import GeminiProvider, Message, OpenAPIToolRegistry, ToolCallingRuntime
from elastisched_api.schemas import LLMChatRequest, LLMChatResponse


llm_router = APIRouter(prefix="/llm", tags=["llm"])


@llm_router.post("/chat", response_model=LLMChatResponse, operation_id="llm_chat")
async def llm_chat(payload: LLMChatRequest, request: Request) -> LLMChatResponse:
    api_key = get_gemini_api_key()
    if not api_key:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="GEMINI_API_KEY is not configured",
        )

    base_url = str(request.base_url).rstrip("/")
    registry = OpenAPIToolRegistry.from_openapi(request.app.openapi(), base_url=base_url)
    provider = GeminiProvider(api_key=api_key, model=get_gemini_model())
    runtime = ToolCallingRuntime(provider, registry, max_steps=payload.max_steps or 6)

    messages = []
    if payload.system:
        messages.append(Message(role="system", content=payload.system))
    messages.append(Message(role="user", content=payload.message))

    try:
        response, _conversation = await runtime.run(messages)
    finally:
        await provider.aclose()
        await registry.aclose()

    return LLMChatResponse(text=response.text, tool_calls=response.tool_calls)
