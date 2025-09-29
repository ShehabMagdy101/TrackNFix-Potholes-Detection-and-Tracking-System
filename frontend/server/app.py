from flask import Flask, jsonify, request, send_file
from flask_cors import CORS
import gspread
from google.oauth2.service_account import Credentials
import os

app = Flask(__name__)
CORS(app)

# ---------- Google Sheets Setup ----------
scope = [
    "https://www.googleapis.com/auth/spreadsheets",
    "https://www.googleapis.com/auth/drive"
]

creds = Credentials.from_service_account_file("credentials.json", scopes=scope)
client = gspread.authorize(creds)

SHEET_ID = "1T9XmjmYaLkm3yOwJNBqXPvIudm69ndpPfEqDWL_e2HY"
sheet = client.open_by_key(SHEET_ID).worksheet("data with status")  # <-- use sheet named "data"

IMAGES_DIR = "images"

# ---------- Helper Function ----------
def process_value(value, dtype="str"):
    """
    Converts and validates value based on the dtype
    dtype: "str", "float", "int"
    """
    if dtype == "float":
        try:
            val = str(value).replace("\n", "").strip()
            return float(val)
        except:
            return None
    elif dtype == "int":
        try:
            val = str(value).replace("\n", "").strip()
            return int(val)
        except:
            return None
    else:  # default str
        return str(value).strip() if value is not None else ""

# ---------- Routes ----------
@app.route("/images/<filename>")
def serve_image(filename):
    path = os.path.join(IMAGES_DIR, filename)
    if os.path.exists(path):
        return send_file(path, mimetype="image/jpeg")
    return jsonify({"error": "Image not found"}), 404

@app.route("/rows", methods=["GET"])
def get_rows():
    rows = sheet.get_all_records()
    processed_rows = []

    for row in rows:
        processed_row = {
            "vehicle_id": process_value(row.get("vehicle_id"), "int"),
            "pothole_id": process_value(row.get("pothole_id"), "int"),
            "latitude": process_value(row.get("latitude"), "float"),
            "longitude": process_value(row.get("longitude"), "float"),
            "time": process_value(row.get("time"), "str"),
            "label": process_value(row.get("label"), "str"),
            "confidence": process_value(row.get("confidence"), "float"),
            "status": process_value(row.get("status"), "str"),
            "image": None
        }

        if "image" in row and row["image"]:
            processed_row["image"] = f"data:image/jpeg;base64,{row['image']}"

        processed_rows.append(processed_row)

    return jsonify(processed_rows)

@app.route("/update_status", methods=["POST"])
def update_status():
    data = request.get_json()
    pothole_id = data.get("pothole_id")
    new_status = data.get("status")

    if pothole_id is None or new_status is None:
        return jsonify({"error": "pothole_id and status required"}), 400

    try:
        headers = sheet.row_values(1)
        if "status" not in headers or "pothole_id" not in headers:
            return jsonify({"error": "Required columns not found"}), 400

        status_col = headers.index("status") + 1
        pothole_id_col = headers.index("pothole_id") + 1

        all_values = sheet.get_all_values()
        updated = False
        for idx, row in enumerate(all_values[1:], start=2):
            if str(row[pothole_id_col - 1]).strip() == str(pothole_id).strip():
                sheet.update_cell(idx, status_col, new_status)
                updated = True
                break

        if updated:
            return jsonify({"message": "Status updated successfully"})
        else:
            return jsonify({"error": "pothole_id not found"}), 404

    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
