from flask import Flask, jsonify, send_file
import pandas as pd
import base64
import io
from PIL import Image
from ultralytics import YOLO
import gspread
from oauth2client.service_account import ServiceAccountCredentials
from flask_apscheduler import APScheduler

app = Flask(__name__)

sched = APScheduler()

def job1():
    print("hello")

# Load YOLO model once at startup
model = YOLO(r"C:\Users\sheha\Desktop\Digitopia\backend\yolo model\best.pt")

# === Google Sheets API setup ===
OUTPUT_SHEET_ID = "1T9XmjmYaLkm3yOwJNBqXPvIudm69ndpPfEqDWL_e2HY"

scope = [
    "https://spreadsheets.google.com/feeds",
    "https://www.googleapis.com/auth/drive",
]
creds = ServiceAccountCredentials.from_json_keyfile_name(
    r"C:\Users\sheha\Desktop\Digitopia\backend\credentials\x-fabric-433611-s2-cab1cb26faa8.json", scope
)
client = gspread.authorize(creds)


# ================== HELPERS ==================
def load_sheet_as_df(sheet_name=None):
    """Load Raw Data from Google Sheet into pandas DataFrame"""
    sheet = client.open_by_key(OUTPUT_SHEET_ID)
    if sheet_name:
        worksheet = sheet.worksheet(sheet_name)
    else:
        worksheet = sheet.sheet1  # default first sheet
    data = worksheet.get_all_records()
    df = pd.DataFrame(data)
    return df


# ================== ROUTES ==================
@app.route("/")
def home():
    return "Server is Running"

@app.route("/sheet")
def get_sheet():
    """Print the first 5 rows from the raw data"""
    try:
        df = load_sheet_as_df()
        print("\n=== Google Sheet Data ===")
        print(df.head())
        print("=========================\n")
        return jsonify(df.to_dict(orient="records"))
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/image/<int:row_id>")
def get_image(row_id):
    """Decode Base64 image from dataset row and return as PNG"""
    try:
        df = load_sheet_as_df()
        base64_str = df.iloc[row_id]["image"]
        img_data = base64.b64decode(base64_str)
        return send_file(io.BytesIO(img_data), mimetype="image/png")
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/predict-and-save", methods=["GET"])
def predict_and_save():
    """Fetch sheet, predict images, and save results into Google Sheet tab 'data classification'"""
    try:
        df = load_sheet_as_df()
        df["label"] = ""
        df["confidence"] = ""

        # keep rows where pothole detected
        pothole_rows = []

        for idx, row in df.iterrows():
            try:
                # Decode image
                img_data = base64.b64decode(row["image"])
                image = Image.open(io.BytesIO(img_data)).convert("RGB")

                # Run YOLO
                results = model.predict(image, conf=0.1, imgsz=640)

                labels, confidences = [], []
                for r in results:
                    for box in r.boxes:
                        labels.append(model.names[int(box.cls)])
                        confidences.append(float(box.conf))

                # Only pothole or no
                if "pothole" in labels:
                    df.at[idx, "label"] = "yes"
                    df.at[idx, "confidence"] = (
                        f'{max(confidences):.4f}' if confidences else "0"
                    )
                
                    pothole_row = df.iloc[idx].to_dict()
                    pothole_row["status"] = "Not Fixed"  # add status column
                    pothole_rows.append(pothole_row)

                else:
                    df.at[idx, "label"] = "no"
                    df.at[idx, "confidence"] = "0"

            except Exception as e:
                df.at[idx, "label"] = "error"
                df.at[idx, "confidence"] = str(e)

        # === Save results into specific tab ===
        sheet = client.open_by_key(OUTPUT_SHEET_ID)
        try:
            worksheet = sheet.worksheet("data classification")
        except gspread.exceptions.WorksheetNotFound:
            worksheet = sheet.add_worksheet(
                title="data classification", rows="100", cols="20"
            )

        worksheet.clear()
        worksheet.update([df.columns.values.tolist()] + df.values.tolist())

        # === Append pothole rows with status into "data with status" sheet ===
        if pothole_rows:
            try:
                ui_ws = sheet.worksheet("data with status")
            except gspread.exceptions.WorksheetNotFound:
                ui_ws = sheet.add_worksheet(title="data with status", rows="100", cols="20")

            ui_df = pd.DataFrame(pothole_rows)

            # If UI data is empty, add headers
            existing_data = ui_ws.get_all_values()
            if not existing_data:
             # Sheet empty → write headers + data
                ui_ws.update([ui_df.columns.values.tolist()] + ui_df.values.tolist())
            else:
             # Keep headers always → clear and rewrite headers + new data
                ui_ws.clear()
                ui_ws.update([ui_df.columns.values.tolist()] + ui_df.values.tolist())


        return jsonify(
            {
                "message": "Predictions saved",
                "classified_link": f"{sheet.url}#gid={worksheet.id}",
                "data_with_status_link": f"{sheet.url}#gid={ui_ws.id}" if pothole_rows else None,
            }
        )

    except Exception as e:
        return jsonify({"error": str(e)}), 500

# ================== RUN ==================
if __name__ == "__main__":
    sched.add_job(id="prediction", func=predict_and_save, trigger="interval", hours=5)
    sched.start()
    app.run(debug=True, use_reloader=False)