<img width="1920" height="1080" alt="Thumbnail" src="https://github.com/user-attachments/assets/1bffda6f-3cd2-4a73-8806-2996c9f9d38b" />


---

- Project Demo Video [here](https://youtu.be/YyVkExRSrEc) ▶️


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

# Dataset

<img width="846" height="265" alt="image" src="https://github.com/user-attachments/assets/e9fb106d-07d1-403a-bd3a-9cee530c3725" />

---

# 🛠️ Tech Stack  

- **Hardware:** ESP32-CAM, NEO-6M GPS
- **Embedded Development:** Arduino IDE
- **Backend:** Python Flask, Node-RED, HiveMQ (MQTT)  
- **Frontend:** ReactJS, Google Sheets API  
- **Machine Learning:** YOLO (trained on Colab)  

<img width="902" height="514" alt="image" src="https://github.com/user-attachments/assets/a9ce9e5d-4194-4049-b4f5-fab8f5cdd6f7" />


---

# ⚙️ Hardware & Embedded System  


- **ESP32-CAM** captures road images when a disturbance is detected.  
- **NEO-6M GPS** records exact pothole location.  
- **Sensor Module** detects vibration/road disturbance to trigger the camera.  
- **Power Supply** designed to work with battery or vehicle power.  
- **Embedded Code** handles real-time detection, MQTT publishing, and WiFi configuration.  

<img width="436" height="489" alt="image" src="https://github.com/user-attachments/assets/e5bff27e-9585-4338-9ee9-f208b59e4594" />


# 💻 Web App Dashboard  


<img width="874" height="444" alt="image" src="https://github.com/user-attachments/assets/7b07956b-7bdb-447a-bb9a-e4dfdb6d9033" />


# 🚀 Future Improvements  

- Enhance ML model accuracy with larger datasets.  
- Develop a **mobile app** for easier community reporting.  
- Implement **real-time notifications** for nearby potholes.  
- Explore **distributed data storage** for large-scale deployments.  
- Add **donation & reward system** to fund and encourage road repairs.  


# 👨‍💻 Team Members & Roles  


| Name                                                                 | Role                                                |  
|----------------------------------------------------------------------|----------------------------------------------------|  
| [Shehab Magdy](https://www.linkedin.com/in/shehab-magdy-se2122003x/) | Team Lead, Embedded System Coding, UML, Presentation |  
| [Abdelrahman Mohammed](https://www.linkedin.com/in/abdulrahman-mohammed2003/) | Machine Learning – Model Training & Evaluation      |  
| [Abdelrahman Saied](https://www.linkedin.com/in/abdelrahman-saied-010b85194/) | Backend Development – Flask & Server Integration    |  
| [Seif Khaled](https://www.linkedin.com/in/seif-khaled-83bb99252/)    | Frontend Development – ReactJS & UI/UX              |  



---

📌 *This project was developed as part of our academic journey at the Digitopia competition 2025.*  


