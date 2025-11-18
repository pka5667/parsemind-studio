from flask import Blueprint, jsonify
import requests
import subprocess

detect_api = Blueprint("detect", __name__)


@detect_api.route("/ollama/status", methods=["GET"])
def ollama_status():
    """Detect Ollama installation by running `ollama --version`, then probe the local API for running/models.

    Returns JSON:
      {"installed": bool, "running": bool, "models": [str, ...]}
    """
    models = []
    installed = False

    # Step 1: detect installation by running `ollama --version`
    try:
        p = subprocess.run(["ollama", "--version"], capture_output=True, text=True, timeout=1)
        if p.returncode == 0:
            installed = True
    except Exception:
        installed = False

    # Step 2: probe the running API to see if service is up and list models
    try:
        r = requests.get("http://127.0.0.1:11434/api/tags", timeout=0.5)
        if r.status_code == 200:
            data = r.json()
            # data may be dict with 'models' key or a list
            possible = []
            if isinstance(data, dict):
                possible = data.get("models") or data.get("tags") or []
            elif isinstance(data, list):
                possible = data

            parsed = []
            for item in possible:
                if isinstance(item, dict):
                    name = item.get("name") or item.get("model") or item.get("id")
                    if name:
                        parsed.append(name)
                elif isinstance(item, str):
                    parsed.append(item)

            models = parsed
            return jsonify({"installed": True, "running": True, "models": models})
    except Exception:
        # API not reachable or returned unexpected data
        pass

    return jsonify({"installed": installed, "running": False, "models": []})
