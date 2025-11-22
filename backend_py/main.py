"""
Backend module for C++ integration.
Provides C-compatible API functions that return JSON strings.
"""
import json
from app.detect import get_ollama_status


def health_check() -> str:
    """Health check function. Returns JSON string."""
    return {"status": "ok"}



def ollama_status() -> str:
    """Get Ollama installation and status. Returns JSON string."""
    try:
        return get_ollama_status()
    except Exception as e:
        print(f"Error getting ollama status: {e}")
        return {
            "installed": False,
            "running": False,
            "models": [],
            "error": str(e)
        }


if __name__ == "__main__":
    print(ollama_status())