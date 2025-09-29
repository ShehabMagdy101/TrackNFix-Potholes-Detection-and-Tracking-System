import { useEffect, useState, useRef } from "react";
import { MapContainer, TileLayer, Marker, Popup } from "react-leaflet";
import "leaflet/dist/leaflet.css";
import { motion } from "framer-motion";
import L from "leaflet";
import './App.css';
import logo from "./TrackNFix_logo.jpg";
import competitionLogo from "./digitopia_Egypt_logo.jpeg";

// Orange Marker Icon (from pointhi repo)
const customIcon = new L.Icon({
  iconUrl: "https://raw.githubusercontent.com/pointhi/leaflet-color-markers/master/img/marker-icon-orange.png",
  iconRetinaUrl: "https://raw.githubusercontent.com/pointhi/leaflet-color-markers/master/img/marker-icon-2x-orange.png",
  shadowUrl: "https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.9.4/images/marker-shadow.png",
  iconSize: [25, 41],
  iconAnchor: [12, 41],
  popupAnchor: [1, -34],
  shadowSize: [41, 41],
});

function App() {
  const [data, setData] = useState([]);
  const [selected, setSelected] = useState(null);
  const [filterStatus, setFilterStatus] = useState("");
  const [minConfidence, setMinConfidence] = useState(0);
  const [startDate, setStartDate] = useState("");
  const [endDate, setEndDate] = useState("");
  const mapRef = useRef(null);

  const fetchData = () => {
    fetch("http://localhost:5000/rows")
      .then((res) => res.json())
      .then((rows) => {
        const mappedData = rows
          .map((row, index) => ({
            id: row.pothole_id || index,
            lat: parseFloat(row.latitude),
            lng: parseFloat(row.longitude),
            img: row.image || null,
            status: row.status || "pending",
            confidence: row.confidence !== undefined ? parseFloat(row.confidence) : 0,
            time: row.time || null,
          }))
          .filter((point) => !isNaN(point.lat) && !isNaN(point.lng));
        setData(mappedData);
      })
      .catch((err) => console.error("Error fetching rows:", err));
  };

  useEffect(() => {
    fetchData();
  }, []);

  const updateStatus = (id, newStatus) => {
    if (selected && selected.id === id) {
      setSelected({ ...selected, status: newStatus });
    }
    fetch("http://localhost:5000/update_status", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ pothole_id: id, status: newStatus }),
    })
      .then((res) => {
        if (!res.ok) throw new Error("Failed to update status");
        return res.json();
      })
      .then(() => fetchData())
      .catch((err) => {
        console.error("Error updating status:", err);
        fetchData();
      });
  };

  const getConfidenceColor = (conf) => {
    if (conf > 0.7) return "#5cb85c";
    if (conf >= 0.4) return "#f0ad4e";
    return "#d9534f";
  };

  // Apply filters (including date range)
  const filteredData = data
    .filter((d) => !filterStatus || d.status === filterStatus)
    .filter((d) => (d.confidence || 0) >= minConfidence)
    .filter((d) => {
      if (!startDate && !endDate) return true;
      if (!d.time) return false;
      const pointTime = new Date(d.time);
      if (startDate) {
        const start = new Date(startDate);
        start.setHours(0, 0, 0, 0);
        if (pointTime < start) return false;
      }
      if (endDate) {
        const end = new Date(endDate);
        end.setHours(23, 59, 59, 999);
        if (pointTime > end) return false;
      }
      return true;
    });

  return (
    <div className="App">
      {/* Navbar */}
      <header className="navbar">
        <div className="logo-section">
          <img src={logo} alt="TrackNFix Logo" className="logo" />
        </div>
        <div className="competition-logo">
          <img src={competitionLogo} alt="Competition Logo" className="comp-logo" />
        </div>
      </header>

      {/* Controls: Filters + Stats */}
      <section className="controls-section">
        <div className="filters">
          <div className="filter-card">
            <label>Status</label>
            <select onChange={(e) => setFilterStatus(e.target.value)} value={filterStatus}>
              <option value="">All</option>
              <option value="pending">Pending</option>
              <option value="reviewed">Reviewed</option>
              <option value="ignored">Ignored</option>
              <option value="fixed">Fixed</option>
            </select>
          </div>

          <div className="filter-card">
            <label>Confidence ≥</label>
            <input
              type="range"
              min="0"
              max="1"
              step="0.01"
              value={minConfidence}
              onChange={(e) => setMinConfidence(parseFloat(e.target.value))}
            />
            <span className="confidence-value">{minConfidence}</span>
          </div>

          <div className="filter-card">
            <label>Date From</label>
            <input type="date" value={startDate} onChange={(e) => setStartDate(e.target.value)} />
          </div>

          <div className="filter-card">
            <label>Date To</label>
            <input type="date" value={endDate} onChange={(e) => setEndDate(e.target.value)} />
          </div>

          <div className="filter-card">
            <label>&nbsp;</label>
            <button
              className="clear-btn"
              onClick={() => {
                setFilterStatus("");
                setMinConfidence(0);
                setStartDate("");
                setEndDate("");
              }}
            >
              Clear Filters
            </button>
          </div>
        </div>

        <div className="stats-bar">
          <div className="stats-ticket ticket-total">Total: {filteredData.length}</div>
          <div className="stats-ticket ticket-pending">
            Pending: {filteredData.filter((d) => d.status === "pending").length}
          </div>
          <div className="stats-ticket ticket-fixed">
            Fixed: {filteredData.filter((d) => d.status === "fixed").length}
          </div>
          <div className="stats-ticket ticket-reviewed">
            Reviewed: {filteredData.filter((d) => d.status === "reviewed").length}
          </div>
        </div>
      </section>

      {/* Map + Sidebar */}
      <div className="main-container" ref={mapRef}>
        <div className="map-container">
          <MapContainer
            center={[30.0444, 31.2357]}
            zoom={6}
            style={{ height: "100%", width: "100%" }}
          >
            <TileLayer
              url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
              attribution="&copy; OpenStreetMap contributors"
            />
            {filteredData.map((point) => (
              <Marker
                key={point.id}
                position={[point.lat, point.lng]}
                icon={customIcon}
                eventHandlers={{
                  click: () => setSelected(point),
                }}
              >
                <Popup>
                  <div style={{ minWidth: 160 }}>
                    <strong>Status:</strong> {point.status}
                    <br />
                    <strong>Confidence:</strong>{" "}
                    <span style={{ color: getConfidenceColor(point.confidence) }}>
                      {Math.round((point.confidence || 0) * 100)}%
                    </span>
                  </div>
                </Popup>
              </Marker>
            ))}
          </MapContainer>
        </div>

        <motion.div
          className="sidebar"
          initial={{ x: 300 }}
          animate={{ x: 0 }}
          transition={{ type: "spring", stiffness: 80 }}
        >
          <h2>Selected Image Details</h2>
          {!selected && <p>Click a marker to view details here.</p>}
          {selected && (
            <div className="selected-details">
              {selected.img && (
                <img src={selected.img} alt="location" className="image-preview" />
              )}
              <div className="info">
                <p>
                  <strong>Latitude:</strong> {selected.lat}
                </p>
                <p>
                  <strong>Longitude:</strong> {selected.lng}
                </p>
                <p>
                  <strong>Time:</strong>{" "}
                  {selected.time
                    ? new Date(selected.time).toLocaleString("en-GB", {
                        day: "2-digit",
                        month: "short",
                        year: "numeric",
                        hour: "2-digit",
                        minute: "2-digit",
                        hour12: true,
                      })
                    : "N/A"}
                </p>

                <p>
                  <strong>Status:</strong>
                </p>
                <div className="selected-details-buttons">
                  <button
                    className="fixed-btn"
                    onClick={() => updateStatus(selected.id, "fixed")}
                  >
                    Fixed
                  </button>
                  <button
                    className="review-btn"
                    onClick={() => updateStatus(selected.id, "reviewed")}
                  >
                    Reviewed
                  </button>
                  <select
                    className="status-dropdown"
                    value={selected.status}
                    onChange={(e) => updateStatus(selected.id, e.target.value)}
                  >
                    <option value="pending">Pending</option>
                    <option value="reviewed">Reviewed</option>
                    <option value="ignored">Ignored</option>
                    <option value="fixed">Fixed</option>
                  </select>
                </div>

                <p>
                  <strong>Confidence:</strong>
                  <span
                    style={{
                      display: "inline-block",
                      width: `${(selected.confidence || 0) * 100}%`,
                      height: "12px",
                      backgroundColor: getConfidenceColor(selected.confidence || 0),
                      borderRadius: "4px",
                      marginLeft: "10px",
                    }}
                  ></span>
                  {` ${Math.round((selected.confidence || 0) * 100)}%`}
                </p>
              </div>
            </div>
          )}
        </motion.div>
      </div>

      <footer
        style={{
          textAlign: "center",
          padding: "15px 0",
          backgroundColor: "#252c2c",
          color: "#ea7f30",
        }}
      >
        © 2025 TrackNFix. All rights reserved.
      </footer>
    </div>
  );
}

export default App;
