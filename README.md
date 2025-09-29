<img width="1920" height="1080" alt="Thumbnail" src="https://github.com/user-attachments/assets/1bffda6f-3cd2-4a73-8806-2996c9f9d38b" />


# 📑 Table of Contents
- [Overview](#overview)  
- [Features](#features)  
- [System Architecture](#system-architecture)  
- [Tech Stack](#tech-stack)  
- [Hardware & Embedded System](#hardware--embedded-system)  
- [Web App Dashboard](#web-app-dashboard)  
- [Future Improvements](#future-improvements)  
- [Team Members & Roles](#team-members--roles)  

---

# 🔎 Overview
Poor road conditions and potholes lead to unsafe driving, increased vehicle damage, and higher accident risks. Traditional reporting methods are slow and often inefficient.  

**TrackNFix** addresses this problem by:  
- Detecting potholes automatically using sensors and AI.  
- Capturing GPS coordinates and camera evidence.  
- Uploading pothole data to the cloud.  
- Providing a web dashboard for monitoring and tracking.  

---

# ✨ Features  

- 📷 **Automatic Pothole Detection** using ESP32-CAM and vibration sensors.  
- 📍 **GPS Location Tracking** with NEO-6M module.  
- 🧠 **AI-Powered Pothole Recognition** (YOLO model with ~87% accuracy).  
- ☁️ **Cloud Integration** via MQTT and Google Sheets API.  
- 📊 **Web Dashboard** for visualization, reporting, and analytics.  
- 👥 **Community Engagement**: Users can report potholes manually.  
- 🔔 **Status Tracking**: Authorities and users can update pothole repair progress.  

---

# 🏗️ System Architecture  

<img width="1106" height="646" alt="system architecture" src="https://github.com/user-attachments/assets/ef821b79-db46-43a5-b71b-fa2f9153c385" />


The system consists of:  
1. **IoT Device**: ESP32-CAM + GPS to detect potholes and send data.  
2. **Machine Learning Model**: YOLO for pothole image classification.  
3. **Backend Server**: Flask + Node-RED + HiveMQ for data processing.  
4. **Frontend Dashboard**: ReactJS + Google Sheets API for visualization.  
5. **Database/Warehouse**: Google Sheets for storage and historical analysis.  


<img width="1738" height="428" alt="deployement diagram" src="https://github.com/user-attachments/assets/c97f4eff-2099-4917-847c-9398a9769aa4" />


---
# Database Design

<img width="1290" height="471" alt="class diagram" src="https://github.com/user-attachments/assets/d8d3c045-4e3f-45ec-9ccf-9100841df55d" />

<img width="846" height="265" alt="image" src="https://github.com/user-attachments/assets/e9fb106d-07d1-403a-bd3a-9cee530c3725" />

---

# 🛠️ Tech Stack  

**Hardware:** ESP32-CAM, NEO-6M GPS, vibration sensors  
**Embedded Development:** Arduino IDE, PlatformIO  
**Backend:** Python Flask, Node-RED, HiveMQ (MQTT)  
**Frontend:** ReactJS, Google Sheets API  
**Machine Learning:** YOLO (trained on Colab)  
**Data & Visualization:** Google Sheets, Grafana  

---

# ⚙️ Hardware & Embedded System  

<img width="433" height="577" alt="image-removebg-preview (1)" src="https://github.com/user-attachments/assets/ba8d83c9-5bed-45ed-b2a8-143d48fa3a0c" />


- **ESP32-CAM** captures road images when a disturbance is detected.  
- **NEO-6M GPS** records exact pothole location.  
- **Sensor Module** detects vibration/road disturbance to trigger the camera.  
- **Power Supply** designed to work with battery or vehicle power.  
- **Embedded Code** handles real-time detection, MQTT publishing, and WiFi configuration.  

<img width="436" height="489" alt="image" src="https://github.com/user-attachments/assets/e5bff27e-9585-4338-9ee9-f208b59e4594" />


# 💻 Web App Dashboard  

<img width="666" height="428" alt="image" src="https://github.com/user-attachments/assets/8e4d0437-9016-4e06-b2d1-a49d17e194b1" />

<img width="874" height="444" alt="image" src="https://github.com/user-attachments/assets/7b07956b-7bdb-447a-bb9a-e4dfdb6d9033" />


# 🚀 Future Improvements  

- Enhance ML model accuracy with larger datasets.  
- Develop a **mobile app** for easier community reporting.  
- Implement **real-time notifications** for nearby potholes.  
- Explore **distributed data storage** for large-scale deployments.  
- Add **donation & reward system** to fund and encourage road repairs.  


# 👨‍💻 Team Members & Roles  

| Name        | Role                                   |  
|-------------|----------------------------------------|  
| Shehab Magdy | Team Lead, Embedded System Coding, UML, Presentation |  
| Abdelrahman Mohammed | Machine Learning – Model Training & Evaluation |  
| Abdelrahman Saied  | Backend Development – Flask & Server Integration |  
| Seif Khaled  | Frontend Development – ReactJS & UI/UX |  


---

📌 *This project was developed as part of our academic journey at the Digitopia Compition 2025.*  
