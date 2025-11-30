import requests
import subprocess
import shutil
import os
import traceback
import datetime


def get_ollama_status():
    """
    Detect Ollama installation and running status.

    Strategy:
      1. Use `shutil.which('ollama')` to check if the binary is on PATH.
      2. If not found, try running `ollama --version` (longer timeout).
      3. If installed, probe the local API at 127.0.0.1:11434 to list models.

    Returns a plain Python dict:

      {
        "installed": bool,
        "running": bool,
        "models": [str, ...]
      }
    """
    models = []
    installed = False
    running = False
    debug_lines = []

    # Step 1: prefer checking PATH (fast and reliable)
    try:
        exe_path = shutil.which("ollama")
        debug_lines.append(f"which(ollama)={exe_path}")
        if exe_path:
            installed = True
        else:
            # fallback: attempt to run the binary (capture output for debugging)
            try:
                p = subprocess.run(
                    ["ollama", "--version"],
                    capture_output=True,
                    text=True,
                    timeout=3,
                )
                debug_lines.append(f"run.returncode={p.returncode} stdout={p.stdout.strip()} stderr={p.stderr.strip()}")
                if p.returncode == 0:
                    installed = True
            except Exception:
                debug_lines.append("exec_exception:" + traceback.format_exc())
                installed = False
    except Exception:
        debug_lines.append("which_exception:" + traceback.format_exc())
        installed = False

    # Step 2: probe the running API to see if service is up and list models
    if installed:
        try:
            # allow slightly more time for the API to respond
            r = requests.get("http://127.0.0.1:11434/api/tags", timeout=1)
            debug_lines.append(f"api_status={getattr(r, 'status_code', 'noresp')}")
            if r.status_code == 200:
                data = r.json()
                possible = []
                if isinstance(data, dict):
                    possible = data.get("models") or data.get("tags") or []
                elif isinstance(data, list):
                    possible = data

                parsed = []
                for item in possible:
                    if isinstance(item, dict):
                        name = (
                            item.get("name")
                            or item.get("model")
                            or item.get("id")
                        )
                        if name:
                            parsed.append(name)
                    elif isinstance(item, str):
                        parsed.append(item)

                models = parsed
                running = True
        except Exception:
            debug_lines.append("api_exception:" + traceback.format_exc())
            running = False

    # Richer debug logging for intermittent problems
    try:
        log_path = os.path.join(os.getcwd(), "ollama_debug.log")
        with open(log_path, "a", encoding="utf-8") as f:
            f.write(
                f"{datetime.datetime.now().isoformat()} | CWD={os.getcwd()} | PATH={os.environ.get('PATH')} | "
                f"installed={installed} running={running} models={models} | debug={' || '.join(debug_lines)}\n"
            )
    except Exception:
        # don't fail detection if logging fails
        pass

    return {"installed": installed, "running": running, "models": models}
