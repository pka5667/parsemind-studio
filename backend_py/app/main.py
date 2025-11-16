from flask import Flask, jsonify

app = Flask(__name__)

@app.route("/health", methods=["GET"])
def health_check():
    return jsonify({"status": "ok", "version": "0.1", "message": "Flask backend running fine"})

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=8001, debug=False)
