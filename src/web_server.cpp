// File: src/web_server.cpp
#include "globals.h"
#include "web_server.h"
#include "mqtt_manager.h"
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Updater.h>
#include <Wire.h>

ESP8266WebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>DIY Smart Weather Station 2026</title>
    <!-- Favicon -->
    <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'><circle cx='24' cy='24' r='12' fill='%23f59e0b'/><path fill='%2306b6d4' d='M46 28a14 14 0 0 0-26.6-4.5A12 12 0 0 0 8 34a12 12 0 0 0 12 12h26a10 10 0 0 0 0-20z'/></svg>">
    <!-- Google Fonts Outfit -->
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&family=JetBrains+Mono:wght@400;700&display=swap" rel="stylesheet">
    <!-- Material Design Icons CDN -->
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@mdi/font@7.4.47/css/materialdesignicons.min.css">
    <style>
        :root {
            --bg-grad: radial-gradient(circle at top, #0f172a, #020617);
            --card-bg: rgba(30, 41, 59, 0.7);
            --card-border: rgba(255, 255, 255, 0.08);
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --primary: #06b6d4;
            --primary-glow: rgba(6, 182, 212, 0.3);
            --success: #10b981;
            --success-glow: rgba(16, 185, 129, 0.3);
            --warning: #f59e0b;
            --danger: #ef4444;
            --danger-glow: rgba(239, 68, 68, 0.3);
            --console-bg: #030712;
            --console-text: #34d399;
        }

        /* Compact Board Single-Container Layout */
        .compact-board {
            background: var(--card-bg);
            border: 1px solid var(--card-border);
            border-radius: 20px;
            padding: 16px;
            backdrop-filter: blur(16px);
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.4);
            max-width: 900px;
            margin: 0 auto 20px auto;
        }

        .c-board-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding-bottom: 12px;
            border-bottom: 1px solid var(--card-border);
            margin-bottom: 14px;
            flex-wrap: wrap;
            gap: 8px;
        }

        .c-title {
            display: flex;
            align-items: center;
            gap: 8px;
            font-weight: 800;
            font-size: 1.15rem;
            color: var(--text-main);
        }

        .c-badge-ver {
            font-size: 0.7rem;
            background: rgba(6, 182, 212, 0.15);
            color: var(--primary);
            padding: 2px 6px;
            border-radius: 6px;
            border: 1px solid rgba(6, 182, 212, 0.3);
        }

        .c-header-badges {
            display: flex;
            gap: 8px;
            align-items: center;
        }

        .c-status-badge {
            font-size: 0.75rem;
            padding: 4px 10px;
            border-radius: 12px;
            font-weight: 600;
            display: inline-flex;
            align-items: center;
            gap: 4px;
        }

        .badge-info {
            background: rgba(255, 255, 255, 0.08);
            color: var(--text-muted);
            border: 1px solid var(--card-border);
        }

        .c-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 10px;
        }

        @media (min-width: 600px) {
            .c-grid {
                grid-template-columns: repeat(4, 1fr);
            }
        }

        .c-tile {
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid var(--card-border);
            border-radius: 12px;
            padding: 10px 12px;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .c-tile.c-span-2 {
            grid-column: span 2;
        }

        .c-tile-icon {
            font-size: 1.5rem;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .c-tile-content {
            flex: 1;
            min-width: 0;
        }

        .c-tile-label {
            font-size: 0.68rem;
            font-weight: 700;
            color: var(--text-muted);
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        .c-tile-val {
            font-size: 1.15rem;
            font-weight: 800;
            color: var(--text-main);
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            line-height: 1.2;
        }

        .c-unit {
            font-size: 0.75rem;
            color: var(--text-muted);
            font-weight: 400;
        }

        .c-tile-sub {
            font-size: 0.68rem;
            color: var(--text-muted);
            margin-top: 1px;
        }

        .c-board-footer {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding-top: 10px;
            border-top: 1px solid var(--card-border);
            margin-top: 12px;
            font-size: 0.75rem;
            color: var(--text-muted);
            flex-wrap: wrap;
            gap: 8px;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Outfit', sans-serif;
            background: var(--bg-grad);
            background-attachment: fixed;
            color: var(--text-main);
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            max-width: 900px;
            margin: 0 auto;
        }

        header {
            text-align: center;
            margin-bottom: 30px;
            padding-top: 10px;
        }

        header h1 {
            font-size: 2.2rem;
            font-weight: 800;
            background: linear-gradient(135deg, #06b6d4, #3b82f6);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 5px;
            letter-spacing: -0.5px;
        }

        header p {
            color: var(--text-muted);
            font-size: 0.95rem;
        }

        /* Nav Tabs */
        .nav-tabs {
            display: flex;
            justify-content: center;
            gap: 10px;
            margin-bottom: 25px;
            background: rgba(15, 23, 42, 0.6);
            padding: 6px;
            border-radius: 30px;
            border: 1px solid var(--card-border);
        }

        .tab-btn {
            background: transparent;
            border: none;
            color: var(--text-muted);
            padding: 10px 24px;
            border-radius: 20px;
            cursor: pointer;
            font-weight: 600;
            font-size: 0.95rem;
            transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
        }

        @media (max-width: 600px) {
            body {
                padding: 10px;
            }
            header h1 {
                font-size: 1.6rem;
            }
            .nav-tabs {
                gap: 4px;
                padding: 4px;
                border-radius: 16px;
                width: 100%;
                overflow-x: auto;
                justify-content: space-between;
            }
            .tab-btn {
                padding: 8px 8px;
                font-size: 0.8rem;
                border-radius: 12px;
                flex: 1;
                text-align: center;
                white-space: nowrap;
            }
            .tab-btn i {
                font-size: 0.85rem;
                margin-right: 2px;
            }
        }

        .tab-btn.active {
            background: var(--primary);
            color: #000;
            box-shadow: 0 0 15px var(--primary-glow);
        }

        .tab-content {
            display: none;
        }

        .tab-content.active {
            display: block;
            animation: fadeIn 0.4s ease-out;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }

        /* Cards Grid */
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 25px;
        }

        .card {
            background: var(--card-bg);
            border: 1px solid var(--card-border);
            border-radius: 16px;
            padding: 24px;
            text-align: center;
            box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.3);
            backdrop-filter: blur(12px);
            position: relative;
            overflow: hidden;
            transition: transform 0.3s ease;
        }

        .card:hover {
            transform: translateY(-2px);
        }

        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 4px;
            background: transparent;
        }

        .card.primary::before { background: var(--primary); }
        .card.success::before { background: var(--success); }
        .card.warning::before { background: var(--warning); }
        .card.danger::before { background: var(--danger); }

        .card-title {
            color: var(--text-muted);
            font-size: 0.85rem;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 12px;
        }

        .card-value {
            font-size: 2.2rem;
            font-weight: 800;
            margin-bottom: 6px;
        }

        .card-unit {
            font-size: 1rem;
            color: var(--text-muted);
            font-weight: 400;
        }

        .status-badge {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            padding: 8px 16px;
            border-radius: 20px;
            font-weight: 600;
            font-size: 1rem;
            margin-top: 5px;
        }

        .badge-clear {
            background: rgba(16, 185, 129, 0.15);
            color: var(--success);
            border: 1px solid rgba(16, 185, 129, 0.3);
            box-shadow: 0 0 10px rgba(16, 185, 129, 0.1);
        }

        .badge-rain {
            background: rgba(6, 182, 212, 0.15);
            color: var(--primary);
            border: 1px solid rgba(6, 182, 212, 0.3);
            box-shadow: 0 0 15px var(--primary-glow);
            animation: pulse 1.8s infinite alternate;
        }

        @keyframes pulse {
            0% { transform: scale(1); box-shadow: 0 0 5px var(--primary-glow); }
            100% { transform: scale(1.03); box-shadow: 0 0 20px var(--primary-glow); }
        }

        /* Buttons & Forms */
        button.btn {
            background: var(--primary);
            color: #020617;
            border: none;
            padding: 12px 24px;
            border-radius: 12px;
            cursor: pointer;
            font-weight: 600;
            font-size: 0.95rem;
            transition: all 0.2s ease;
            box-shadow: 0 4px 12px var(--primary-glow);
        }

        button.btn:hover {
            opacity: 0.9;
            transform: translateY(-1px);
        }

        button.btn-danger {
            background: var(--danger);
            color: #fff;
            box-shadow: 0 4px 12px var(--danger-glow);
        }

        button.btn-sec {
            background: rgba(255, 255, 255, 0.08);
            color: var(--text-main);
            border: 1px solid var(--card-border);
            box-shadow: none;
        }

        button.btn-sec:hover {
            background: rgba(255, 255, 255, 0.15);
        }

        /* Config Form Panel */
        .panel {
            background: var(--card-bg);
            border: 1px solid var(--card-border);
            border-radius: 16px;
            padding: 30px;
            backdrop-filter: blur(12px);
            margin-bottom: 25px;
        }

        .panel-section {
            margin-bottom: 30px;
        }

        .panel-section:last-child {
            margin-bottom: 0;
        }

        .panel-section h3 {
            font-size: 1.2rem;
            margin-bottom: 20px;
            color: var(--primary);
            border-bottom: 1px solid var(--card-border);
            padding-bottom: 8px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .form-group {
            margin-bottom: 18px;
            display: flex;
            flex-direction: column;
            gap: 6px;
        }

        @media (min-width: 600px) {
            .form-group {
                flex-direction: row;
                justify-content: space-between;
                align-items: center;
            }
        }

        label {
            font-size: 0.95rem;
            color: var(--text-muted);
            font-weight: 500;
        }

        input[type="text"], input[type="password"], input[type="number"], select {
            background: rgba(15, 23, 42, 0.8);
            border: 1px solid var(--card-border);
            border-radius: 8px;
            color: #fff;
            padding: 10px 14px;
            width: 100%;
            font-family: inherit;
            font-size: 0.95rem;
            outline: none;
            transition: border-color 0.2s;
        }

        @media (min-width: 600px) {
            input[type="text"], input[type="password"], input[type="number"], select {
                width: 280px;
            }
        }

        input[type="text"]:focus, input[type="password"]:focus, input[type="number"]:focus, select:focus {
            border-color: var(--primary);
        }

        input:disabled {
            background: rgba(15, 23, 42, 0.4);
            color: #64748b;
            border-color: transparent;
        }

        input[type="checkbox"] {
            width: 20px;
            height: 20px;
            accent-color: var(--primary);
            cursor: pointer;
        }

        input[type="file"] {
            background: rgba(15, 23, 42, 0.8);
            border: 1px solid var(--card-border);
            border-radius: 8px;
            padding: 10px;
            color: var(--text-muted);
            width: 100%;
            cursor: pointer;
        }

        /* Console styling (Tasmota Console) */
        .console-container {
            display: flex;
            flex-direction: column;
            height: calc(100vh - 280px);
            min-height: 350px;
            background: var(--console-bg);
            border: 1px solid var(--card-border);
            border-radius: 16px;
            overflow: hidden;
            box-shadow: 0 15px 30px rgba(0,0,0,0.5);
        }

        .console-header {
            background: #111827;
            padding: 12px 20px;
            font-size: 0.85rem;
            color: var(--text-muted);
            font-weight: 600;
            border-bottom: 1px solid var(--card-border);
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .console-log {
            flex-grow: 1;
            padding: 20px;
            overflow-y: auto;
            font-family: 'JetBrains Mono', monospace;
            font-size: 0.8rem;
            line-height: 1.5;
            color: var(--console-text);
            white-space: pre-wrap;
            scroll-behavior: smooth;
        }

        .console-input-bar {
            display: flex;
            background: #111827;
            padding: 10px;
            border-top: 1px solid var(--card-border);
        }

        .console-input-bar input {
            flex-grow: 1;
            background: #030712 !important;
            border: 1px solid var(--card-border);
            border-radius: 8px;
            color: #fff;
            padding: 12px 16px;
            font-family: 'JetBrains Mono', monospace;
            font-size: 0.95rem;
            outline: none;
        }

        .console-input-bar button {
            background: var(--success);
            color: #000;
            border: none;
            padding: 0 20px;
            margin-left: 10px;
            border-radius: 8px;
            cursor: pointer;
            font-weight: 600;
        }

        /* Table */
        .info-table {
            width: 100%;
            border-collapse: collapse;
            font-size: 0.95rem;
            margin-top: 10px;
        }

        .info-table td {
            padding: 12px 8px;
            border-bottom: 1px solid var(--card-border);
        }

        .info-table tr:last-child td {
            border-bottom: none;
        }

        .info-table td:first-child {
            font-weight: 600;
            color: var(--text-muted);
            width: 45%;
        }

        /* Progress Bar */
        .progress-container {
            background: rgba(15, 23, 42, 0.8);
            border-radius: 10px;
            height: 12px;
            overflow: hidden;
            margin-top: 15px;
            display: none;
            border: 1px solid var(--card-border);
        }

        .progress-bar {
            height: 100%;
            width: 0%;
            background: linear-gradient(90deg, var(--primary), var(--success));
            box-shadow: 0 0 10px var(--primary-glow);
            transition: width 0.2s ease;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>DIY Smart Weather Station 2026</h1>
            <p>ESP8266 Weather Monitor &bull; v<span id="fw_ver_title">--</span></p>
        </header>

        <nav class="nav-tabs">
            <button class="tab-btn active" onclick="switchTab('dashboard')"><i class="mdi mdi-view-dashboard-outline"></i> Dashboard</button>
            <button class="tab-btn" onclick="switchTab('console')"><i class="mdi mdi-console"></i> Console</button>
            <button class="tab-btn" onclick="switchTab('config')"><i class="mdi mdi-cog-outline"></i> Settings</button>
            <button class="tab-btn" onclick="switchTab('info')"><i class="mdi mdi-information-outline"></i> Info</button>
        </nav>

        <!-- DASHBOARD TAB -->
        <div id="dashboard" class="tab-content active">
            <!-- CLASSIC VIEW GRID -->
            <div id="classic_dashboard_grid">
                <div class="grid">
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-thermometer" style="color: #ef4444; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Temperature</div>
                        <div class="card-value" id="val_temp">-- <span class="card-unit">°C</span></div>
                    </div>
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-water-percent" style="color: #06b6d4; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Humidity</div>
                        <div class="card-value" id="val_hum">-- <span class="card-unit">%</span></div>
                    </div>
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-gauge" style="color: #8b5cf6; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Pressure</div>
                        <div class="card-value" id="val_press">-- <span class="card-unit">hPa</span></div>
                    </div>
                </div>

                <div class="grid">
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-air-filter" style="color: #10b981; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Air Quality (AQI)</div>
                        <div class="card-value" id="val_aqi">--</div>
                    </div>
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-molecule-co2" style="color: #f59e0b; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Equivalent CO2 (eCO2)</div>
                        <div class="card-value" id="val_eco2">-- <span class="card-unit">ppm</span></div>
                    </div>
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-molecule" style="color: #ec4899; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> TVOC</div>
                        <div class="card-value" id="val_tvoc">-- <span class="card-unit">ppb</span></div>
                    </div>
                </div>

                <div class="grid">
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-weather-windy" style="color: #3b82f6; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Wind Speed</div>
                        <div class="card-value" id="val_wind_speed">-- <span class="card-unit">km/h</span></div>
                        <div style="font-size: 0.85rem; color: var(--text-muted); margin-top: 4px;" id="val_wind_speed_sub">-- m/s &bull; -- kt</div>
                    </div>
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-weather-tornado" style="color: #f97316; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Max Gust (Today)</div>
                        <div class="card-value" id="val_wind_gust">-- <span class="card-unit">km/h</span></div>
                        <div style="font-size: 0.85rem; color: var(--text-muted); margin-top: 4px;" id="val_wind_gust_sub">-- m/s &bull; -- kt</div>
                    </div>
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-compass-outline" style="color: #14b8a6; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Wind Direction</div>
                        <div class="card-value" id="val_wind_dir">--</div>
                    </div>
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-white-balance-sunny" style="color: #eab308; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Luminosity</div>
                        <div class="card-value" id="val_lux">-- <span class="card-unit">lx</span></div>
                    </div>
                </div>

                <div class="grid">
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-weather-partly-rainy" style="color: #3b82f6; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Last Hour</div>
                        <div class="card-value"><span id="val_hour">0.00</span> <span class="card-unit" id="unit_hour">mm</span></div>
                    </div>
                    <div class="card primary">
                        <div class="card-title"><i class="mdi mdi-weather-pouring" style="color: #2563eb; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Last 24 Hours</div>
                        <div class="card-value"><span id="val_day">0.00</span> <span class="card-unit" id="unit_day">mm</span></div>
                    </div>
                    <div class="card success">
                        <div class="card-title"><i class="mdi mdi-water" style="color: #1d4ed8; font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Total Rain</div>
                        <div class="card-value"><span id="val_total">0.00</span> <span class="card-unit" id="unit_total">mm</span></div>
                    </div>
                </div>

                <div class="grid" style="margin-bottom: 25px;">
                    <div class="card warning" style="grid-column: span 3; display: flex; flex-direction: column; justify-content: center; align-items: center; padding: 20px;">
                        <div class="card-title" style="margin-bottom: 8px;"><i class="mdi mdi-weather-rainy" style="font-size: 1.1rem; vertical-align: -2px; margin-right: 4px;"></i> Rain Status</div>
                        <div id="rain_status_badge" class="status-badge badge-clear">
                            <span id="rain_status_dot" style="width: 10px; height: 10px; border-radius: 50%; background: currentColor;"></span>
                            <span id="rain_status_text">No Rain</span>
                        </div>
                    </div>
                </div>

                <div class="panel">
                    <div class="panel-section">
                        <h3><i class="mdi mdi-chip" style="color: var(--primary); margin-right: 6px;"></i> System Information</h3>
                        <table class="info-table">
                            <tr><td><i class="mdi mdi-clock-outline" style="color: var(--primary); margin-right: 4px;"></i> Local Date/Time</td><td><span id="sys_time">NTP Syncing...</span></td></tr>
                            <tr><td><i class="mdi mdi-wifi" style="color: var(--primary); margin-right: 4px;"></i> Wi-Fi SSID</td><td><span id="sys_ssid">--</span></td></tr>
                            <tr><td><i class="mdi mdi-signal" style="color: var(--primary); margin-right: 4px;"></i> Signal RSSI</td><td><span id="sys_rssi">--</span></td></tr>
                            <tr><td><i class="mdi mdi-ip-network" style="color: var(--primary); margin-right: 4px;"></i> IP Address</td><td><span id="sys_ip">--</span></td></tr>
                            <tr><td><i class="mdi mdi-timer-outline" style="color: var(--primary); margin-right: 4px;"></i> Uptime</td><td><span id="sys_uptime">--</span></td></tr>
                            <tr><td><i class="mdi mdi-counter" style="color: var(--primary); margin-right: 4px;"></i> Total Hall Tips</td><td><span id="sys_tips">0</span></td></tr>
                            <tr><td><i class="mdi mdi-flash" style="color: var(--warning); margin-right: 4px;"></i> Supply Voltage</td><td><span id="sys_vcc">--</span></td></tr>
                            <tr><td><i class="mdi mdi-chip" style="color: var(--success); margin-right: 4px;"></i> Free Memory</td><td><span id="sys_heap">--</span></td></tr>
                        </table>
                    </div>
                </div>
            </div>

            <!-- COMPACT UNIFIED SINGLE-BOARD CONTAINER VIEW -->
            <div id="compact_dashboard_board" class="compact-board" style="display: none;">
                <div class="c-board-header">
                    <div class="c-title">
                        <i class="mdi mdi-weather-partly-cloudy" style="color: var(--primary); font-size: 1.4rem;"></i>
                        <span>Weather Station</span>
                        <span class="c-badge-ver">v<span id="c_val_ver">1.0.4</span></span>
                    </div>
                    <div class="c-header-badges">
                        <span id="c_rain_badge" class="c-status-badge badge-clear">
                            <i class="mdi mdi-weather-rainy"></i> <span id="c_rain_text">No Rain</span>
                        </span>
                        <span class="c-status-badge badge-info">
                            <i class="mdi mdi-clock-outline"></i> <span id="c_val_time">--:--</span>
                        </span>
                    </div>
                </div>

                <div class="c-grid">
                    <div class="c-tile">
                        <div class="c-tile-icon" style="color: #ef4444;"><i class="mdi mdi-thermometer"></i></div>
                        <div class="c-tile-content">
                            <div class="c-tile-label">Temp</div>
                            <div class="c-tile-val" id="c_val_temp">-- <span class="c-unit">°C</span></div>
                        </div>
                    </div>

                    <div class="c-tile">
                        <div class="c-tile-icon" style="color: #06b6d4;"><i class="mdi mdi-water-percent"></i></div>
                        <div class="c-tile-content">
                            <div class="c-tile-label">Humidity</div>
                            <div class="c-tile-val" id="c_val_hum">-- <span class="c-unit">%</span></div>
                        </div>
                    </div>

                    <div class="c-tile">
                        <div class="c-tile-icon" style="color: #8b5cf6;"><i class="mdi mdi-gauge"></i></div>
                        <div class="c-tile-content">
                            <div class="c-tile-label">Pressure</div>
                            <div class="c-tile-val" id="c_val_press">-- <span class="c-unit">hPa</span></div>
                        </div>
                    </div>

                    <div class="c-tile">
                        <div class="c-tile-icon" style="color: #eab308;"><i class="mdi mdi-white-balance-sunny"></i></div>
                        <div class="c-tile-content">
                            <div class="c-tile-label">Luminosity</div>
                            <div class="c-tile-val" id="c_val_lux">-- <span class="c-unit">lx</span></div>
                        </div>
                    </div>

                    <div class="c-tile c-span-2">
                        <div class="c-tile-icon" style="color: #3b82f6;"><i class="mdi mdi-weather-windy"></i></div>
                        <div class="c-tile-content">
                            <div class="c-tile-label">Wind Speed & Gust</div>
                            <div class="c-tile-val">
                                <span id="c_val_wind_speed">-- km/h</span> 
                                <span style="font-size: 0.75rem; color: var(--text-muted); font-weight: normal; margin-left: 4px;" id="c_val_wind_gust">(Gust: --)</span>
                            </div>
                            <div class="c-tile-sub" id="c_val_wind_sub">-- m/s &bull; -- kt</div>
                        </div>
                    </div>

                    <div class="c-tile c-span-2">
                        <div class="c-tile-icon" style="color: #14b8a6;"><i class="mdi mdi-compass-outline"></i></div>
                        <div class="c-tile-content">
                            <div class="c-tile-label">Wind Direction</div>
                            <div class="c-tile-val" id="c_val_wind_dir">--</div>
                        </div>
                    </div>

                    <div class="c-tile c-span-2">
                        <div class="c-tile-icon" style="color: #2563eb;"><i class="mdi mdi-weather-pouring"></i></div>
                        <div class="c-tile-content">
                            <div class="c-tile-label">Rain (1h / 24h / Total)</div>
                            <div class="c-tile-val" style="font-size: 0.95rem;">
                                <span id="c_val_rain_1h">0.00</span> | 
                                <span id="c_val_rain_24h">0.00</span> | 
                                <span id="c_val_rain_total">0.00</span> 
                                <span class="c-unit" id="c_unit_rain">mm</span>
                            </div>
                        </div>
                    </div>

                    <div class="c-tile c-span-2">
                        <div class="c-tile-icon" style="color: #10b981;"><i class="mdi mdi-air-filter"></i></div>
                        <div class="c-tile-content">
                            <div class="c-tile-label">Air Quality (AQI / eCO2 / TVOC)</div>
                            <div class="c-tile-val" style="font-size: 0.9rem;">
                                AQI: <span id="c_val_aqi">--</span> | 
                                <span id="c_val_eco2">--</span> ppm | 
                                <span id="c_val_tvoc">--</span> ppb
                            </div>
                        </div>
                    </div>
                </div>

                <div class="c-board-footer">
                    <div><i class="mdi mdi-wifi" style="color: var(--primary);"></i> <span id="c_val_ssid">--</span> (<span id="c_val_rssi">--</span>)</div>
                    <div><i class="mdi mdi-ip-network" style="color: var(--primary);"></i> <span id="c_val_ip">--</span></div>
                    <div><i class="mdi mdi-timer-outline" style="color: var(--primary);"></i> <span id="c_val_uptime">--</span></div>
                </div>
            </div>
        </div>

        <!-- CONSOLE TAB -->
        <div id="console" class="tab-content">
            <div class="console-container">
                <div class="console-header">
                    <span>Debug Console</span>
                    <button class="btn btn-sec" style="font-size: 0.75rem; padding: 4px 10px; border-radius: 6px;" onclick="clearConsoleLog()">Clear</button>
                </div>
                <div id="console_log" class="console-log">Starting console...</div>
                <form id="console_form" onsubmit="sendConsoleCommand(event)">
                    <div class="console-input-bar">
                        <input type="text" id="console_input" placeholder="Type command (e.g. 'help', 'status')..." autocomplete="off">
                        <button type="submit">Send</button>
                    </div>
                </form>
            </div>
        </div>

        <!-- CONFIG TAB -->
        <div id="config" class="tab-content">
            <div class="panel">
                <form id="config_form" onsubmit="saveConfig(event)">
                    <div class="panel-section">
                        <h3><i class="mdi mdi-tune-vertical" style="color: var(--primary); margin-right: 6px;"></i> General Configuration</h3>
                        <div class="form-group">
                            <label for="conf_host">Hostname (mDNS):</label>
                            <input type="text" id="conf_host">
                        </div>
                        <div class="form-group">
                            <label for="conf_units">Unit System:</label>
                            <select id="conf_units">
                                <option value="metric">Metric (&deg;C, km/h, mm, hPa)</option>
                                <option value="imperial">Imperial (&deg;F, mph, in, inHg)</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label for="conf_ui_mode">Dashboard Layout Mode:</label>
                            <select id="conf_ui_mode" onchange="toggleUiMode(this.value)">
                                <option value="classic">Classic (Spacious Cards)</option>
                                <option value="compact">Compact (All-in-One High Density)</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label for="conf_tz">Timezone (GMT / UTC Offset):</label>
                            <select id="conf_tz">
                                <option value="-12">UTC-12:00 (Baker Island)</option>
                                <option value="-11">UTC-11:00 (Samoa, Niue)</option>
                                <option value="-10">UTC-10:00 (Hawaii, Tahiti)</option>
                                <option value="-9">UTC-09:00 (Alaska)</option>
                                <option value="-8">UTC-08:00 (Pacific Time US/Canada)</option>
                                <option value="-7">UTC-07:00 (Mountain Time US/Canada)</option>
                                <option value="-6">UTC-06:00 (Central Time US/Canada, Mexico)</option>
                                <option value="-5">UTC-05:00 (Eastern Time US/Canada, Bogota)</option>
                                <option value="-4">UTC-04:00 (Atlantic Time, Santiago)</option>
                                <option value="-3">UTC-03:00 (Buenos Aires, Brasilia)</option>
                                <option value="-2">UTC-02:00 (Mid-Atlantic)</option>
                                <option value="-1">UTC-01:00 (Azores, Cape Verde)</option>
                                <option value="0">UTC+00:00 (London, Dublin, Lisbon, GMT)</option>
                                <option value="1">UTC+01:00 (Rome, Paris, Berlin, Madrid - CET)</option>
                                <option value="2">UTC+02:00 (Athens, Cairo, Helsinki, Kyiv - EET)</option>
                                <option value="3">UTC+03:00 (Moscow, Istanbul, Riyadh)</option>
                                <option value="4">UTC+04:00 (Dubai, Baku)</option>
                                <option value="5">UTC+05:00 (Karachi, Tashkent)</option>
                                <option value="6">UTC+06:00 (Dhaka, Almaty)</option>
                                <option value="7">UTC+07:00 (Bangkok, Jakarta, Hanoi)</option>
                                <option value="8">UTC+08:00 (Beijing, Singapore, Perth, Hong Kong)</option>
                                <option value="9">UTC+09:00 (Tokyo, Seoul)</option>
                                <option value="10">UTC+10:00 (Sydney, Melbourne, Guam)</option>
                                <option value="11">UTC+11:00 (Solomon Islands, Noumea)</option>
                                <option value="12">UTC+12:00 (Auckland, Fiji)</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label for="conf_dst">Daylight Saving Time (DST):</label>
                            <select id="conf_dst">
                                <option value="false">Disabled (Standard Time +0h)</option>
                                <option value="true">Enabled (Daylight Saving +1h)</option>
                            </select>
                        </div>
                    </div>

                    <div class="panel-section">
                        <h3><i class="mdi mdi-weather-rainy" style="color: var(--primary); margin-right: 6px;"></i> Rain Gauge Parameters</h3>
                        <div class="form-group">
                            <label for="conf_pin">Sensor GPIO (A3144):</label>
                            <input type="number" id="conf_pin" min="0" max="39">
                        </div>
                        <div class="form-group">
                            <label for="conf_cal">Rain Calibration (mm/tip):</label>
                            <input type="number" id="conf_cal" step="0.0001" min="0">
                        </div>
                        <div class="form-group">
                            <label for="conf_deb">Software Debounce (ms):</label>
                            <input type="number" id="conf_deb" min="0">
                        </div>
                        <div class="form-group">
                            <label>Reset Rain Counter:</label>
                            <button type="button" class="btn btn-sec" style="font-size: 0.85rem; padding: 6px 14px; border-radius: 8px;" onclick="clearCounters()"><i class="mdi mdi-refresh"></i> Reset Total Rain</button>
                        </div>
                    </div>

                    <div class="panel-section">
                        <h3><i class="mdi mdi-thermometer-lines" style="color: var(--primary); margin-right: 6px;"></i> Environmental Sensor Parameters</h3>
                        <div class="form-group">
                            <label for="conf_sda">I2C SDA Pin:</label>
                            <input type="number" id="conf_sda" min="0" max="39">
                        </div>
                        <div class="form-group">
                            <label for="conf_scl">I2C SCL Pin:</label>
                            <input type="number" id="conf_scl" min="0" max="39">
                        </div>
                        <div class="form-group">
                            <label for="conf_i2c_clk">I2C SCL Clock Speed:</label>
                            <select id="conf_i2c_clk">
                                <option value="50">50 kHz (Slow/Long Cable)</option>
                                <option value="100">100 kHz (Standard/Safe)</option>
                                <option value="400">400 kHz (Fast)</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label for="conf_alt">Altitude (meters):</label>
                            <input type="number" id="conf_alt" min="0" max="9000">
                        </div>
                        <div class="form-group">
                            <label for="conf_t_offset">Temperature Offset (&deg;C):</label>
                            <input type="number" id="conf_t_offset" step="0.1" min="-20" max="20">
                        </div>
                        <div class="form-group">
                            <label for="conf_h_offset">Humidity Offset (%):</label>
                            <input type="number" id="conf_h_offset" step="0.1" min="-50" max="50">
                        </div>
                        <div class="form-group">
                            <label for="conf_p_offset">Pressure Offset (hPa):</label>
                            <input type="number" id="conf_p_offset" step="0.1" min="-100" max="100">
                        </div>
                        <div class="form-group">
                            <label for="conf_sens_int">Sensor Read Interval (seconds):</label>
                            <input type="number" id="conf_sens_int" min="1" max="60">
                        </div>
                    </div>

                    <div class="panel-section">
                        <h3><i class="mdi mdi-weather-windy" style="color: var(--primary); margin-right: 6px;"></i> Anemometer (Wind) Parameters</h3>
                        <div class="form-group">
                            <label for="conf_w_pin">Sensor GPIO:</label>
                            <input type="number" id="conf_w_pin" min="0" max="39">
                        </div>
                        <div class="form-group">
                            <label for="conf_w_rad">Anemometer Arm Radius (mm):</label>
                            <input type="number" id="conf_w_rad" min="1" max="1000" oninput="updateLiveWindCal()">
                        </div>
                        <div class="form-group">
                            <label for="conf_w_mag">Number of Magnets:</label>
                            <input type="number" id="conf_w_mag" min="1" max="100" oninput="updateLiveWindCal()">
                        </div>
                        <div class="form-group">
                            <label for="conf_w_fac">Aerodynamic Factor (p):</label>
                            <input type="number" id="conf_w_fac" step="0.1" min="0.1" max="10.0" oninput="updateLiveWindCal()">
                        </div>
                        <div class="form-group">
                            <label for="conf_w_cal">Computed Calibration (km/h per Hz):</label>
                            <input type="number" id="conf_w_cal" step="0.0001" disabled style="background-color: rgba(255, 255, 255, 0.05); cursor: not-allowed;">
                        </div>
                        <div class="form-group" style="flex-direction: column; align-items: stretch; gap: 8px; margin-top: 15px; margin-bottom: 20px;">
                            <label>Wind Calibration Guide & Formula:</label>
                            <div style="border: 1px dashed var(--card-border); border-radius: 8px; padding: 15px; background: rgba(0,0,0,0.2); font-size: 0.9rem; line-height: 1.45;">
                                <p style="margin-bottom: 8px;">The calibration factor <strong>K (km/h per Hz)</strong> is computed automatically based on the physical dimensions of the anemometer:</p>
                                <div style="font-family: monospace; background: rgba(0,0,0,0.3); padding: 8px 12px; border-radius: 6px; text-align: center; margin-bottom: 10px; font-size: 0.95rem; border: 1px solid var(--card-border);">
                                    K = (7.2 * &pi; * Radius_mm / 1000 * Factor_p) / Magnets
                                </div>
                                <ul style="padding-left: 20px; color: var(--text-muted); display: flex; flex-direction: column; gap: 4px;">
                                    <li><strong>Radius (mm):</strong> Distance from the center of rotation to the center of any cup (default: 80mm).</li>
                                    <li><strong>Magnets:</strong> Number of pulses per rotation. Set to 2 if adding a second magnet (doubles precision).</li>
                                    <li><strong>Factor p:</strong> Aerodynamic ratio between wind speed and cup linear speed (default: 3.0).</li>
                                </ul>
                            </div>
                        </div>
                        <div class="form-group">
                            <label for="conf_w_deb">Software Debounce (ms):</label>
                            <input type="number" id="conf_w_deb" min="0">
                        </div>
                        <div class="form-group">
                            <label for="conf_w_spd_int">Wind Speed Sample Interval (seconds):</label>
                            <input type="number" id="conf_w_spd_int" min="1" max="60">
                        </div>
                        <div class="form-group">
                            <label for="conf_w_spd_avg">Speed Smoothing (samples, 1&ndash;60):</label>
                            <input type="number" id="conf_w_spd_avg" min="1" max="60">
                        </div>
                        <div class="form-group">
                            <label for="conf_w_dir_avg">Direction Smoothing (samples, 1&ndash;60):</label>
                            <input type="number" id="conf_w_dir_avg" min="1" max="60">
                        </div>
                        <div class="form-group">
                            <label for="conf_w_dir_off">Wind Direction Offset (North calibration, 0-359&deg;):</label>
                            <div style="display: flex; gap: 10px; width: 100%;">
                                <input type="number" id="conf_w_dir_off" min="0" max="359" style="flex: 1; margin: 0;">
                                <button type="button" class="btn btn-sec" onclick="calibrateNorth()" style="margin: 0; padding: 0 15px; font-size: 0.85rem; height: 38px; border-radius: 6px; white-space: nowrap;">Calibrate North</button>
                            </div>
                        </div>
                    </div>

                    <div class="panel-section">
                        <h3><i class="mdi mdi-white-balance-sunny" style="color: var(--primary); margin-right: 6px;"></i> Luminosity Calibration (BH1750 behind PETG)</h3>
                        <div class="form-group">
                            <label for="conf_lux_cal">Transmission Calibration Factor (0.01 - 1.0):</label>
                            <input type="number" id="conf_lux_cal" step="0.0001" min="0.01" max="1.0">
                        </div>
                        <div class="form-group">
                            <label>Guided Transmission Calibration Wizard:</label>
                            <div style="border: 1px dashed var(--card-border); border-radius: 8px; padding: 15px; background: rgba(0,0,0,0.2); font-size: 0.9rem;">
                                <div style="margin-bottom: 15px;">
                                    <strong>Step 1: Unfiltered Reading</strong>
                                    <p style="color: var(--text-muted); font-size: 0.85rem; margin-top: 3px; margin-bottom: 8px;">Expose the lux sensor directly to constant light without the PETG cover.</p>
                                    <div style="display: flex; align-items: center; gap: 10px;">
                                        <button type="button" class="btn btn-sec" style="margin: 0; font-size: 0.85rem;" onclick="readUnfiltered()">Read Unfiltered</button>
                                        <span id="cal_unfiltered_status" style="color: var(--text-muted); font-style: italic;">No reference reading</span>
                                    </div>
                                </div>
                                <div>
                                    <strong>Step 2: Filtered Reading & Calibration</strong>
                                    <p style="color: var(--text-muted); font-size: 0.85rem; margin-top: 3px; margin-bottom: 8px;">Place the PETG cover back on the sensor under the same light source.</p>
                                    <div style="display: flex; align-items: center; gap: 10px;">
                                        <button type="button" class="btn btn-sec" id="btn_cal_filtered" style="margin: 0; font-size: 0.85rem;" onclick="readFiltered()" disabled>Read Filtered</button>
                                        <span id="cal_filtered_status" style="color: var(--text-muted); font-style: italic;">Awaiting Step 1</span>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="panel-section">
                        <h3><i class="mdi mdi-wifi" style="color: var(--primary); margin-right: 6px;"></i> Network Configuration</h3>
                        <div class="form-group" style="flex-direction: row; justify-content: space-between;">
                            <label for="conf_dhcp">Use DHCP:</label>
                            <input type="checkbox" id="conf_dhcp" onchange="toggleDhcpFields(this.checked)">
                        </div>
                        <div class="form-group">
                            <label for="conf_ip">Static IP:</label>
                            <input type="text" id="conf_ip">
                        </div>
                        <div class="form-group">
                            <label for="conf_gw">Gateway:</label>
                            <input type="text" id="conf_gw">
                        </div>
                        <div class="form-group">
                            <label for="conf_nm">Netmask:</label>
                            <input type="text" id="conf_nm">
                        </div>
                        <div class="form-group">
                            <label for="conf_dns_p">Primary DNS:</label>
                            <input type="text" id="conf_dns_p">
                        </div>
                        <div class="form-group">
                            <label for="conf_dns_s">Secondary DNS:</label>
                            <input type="text" id="conf_dns_s">
                        </div>
                        <div class="form-group">
                            <label for="conf_ntp">NTP Server:</label>
                            <input type="text" id="conf_ntp">
                        </div>
                    </div>

                    <div class="panel-section">
                        <h3><i class="mdi mdi-bug-outline" style="color: var(--primary); margin-right: 6px;"></i> Crash Diagnostics</h3>
                        <div class="form-group" style="flex-direction: row; justify-content: space-between; align-items: center;">
                            <label for="conf_crash_opt">Send crash dump (no personal information will be sent):</label>
                            <input type="checkbox" id="conf_crash_opt" style="width: 20px; height: 20px; margin: 0;">
                        </div>
                    </div>

                    <div class="panel-section">
                        <h3><i class="mdi mdi-cloud-upload-outline" style="color: var(--primary); margin-right: 6px;"></i> MQTT Broker</h3>
                        <div class="form-group">
                            <label for="conf_mq">MQTT Server:</label>
                            <input type="text" id="conf_mq" placeholder="e.g. 192.168.1.50">
                        </div>
                        <div class="form-group">
                            <label for="conf_m_port">MQTT Port:</label>
                            <input type="number" id="conf_m_port">
                        </div>
                        <div class="form-group">
                            <label for="conf_m_user">MQTT User:</label>
                            <input type="text" id="conf_m_user">
                        </div>
                        <div class="form-group">
                            <label for="conf_m_pass">MQTT Password:</label>
                            <input type="password" id="conf_m_pass">
                        </div>
                        <div class="form-group">
                            <label for="conf_mqtt_int">MQTT Publish Interval (seconds):</label>
                            <input type="number" id="conf_mqtt_int" min="5" max="3600">
                        </div>
                        <div class="form-group">
                            <label for="conf_mqtt_dec">MQTT Decimal Places:</label>
                            <select id="conf_mqtt_dec">
                                <option value="0">0 (Integer)</option>
                                <option value="1">1 Decimal Place</option>
                                <option value="2">2 Decimal Places</option>
                                <option value="3">3 Decimal Places</option>
                            </select>
                        </div>
                    </div>

                    <div style="text-align: center; margin-top: 20px;">
                        <button type="submit" class="btn" style="width: 100%; max-width: 300px;"><i class="mdi mdi-content-save"></i> Save & Apply</button>
                    </div>
                </form>
            </div>

            <div class="panel">
                <div class="panel-section">
                    <h3><i class="mdi mdi-cloud-download-outline" style="color: var(--primary); margin-right: 6px;"></i> Firmware Update (OTA)</h3>
                    <p style="color: var(--text-muted); font-size: 0.9rem; margin-bottom: 15px;">Select the compiled .bin file to update the weather station.</p>
                    <div style="display: flex; flex-direction: column; gap: 10px;">
                        <input type="file" id="ota_file" accept=".bin">
                        <button class="btn btn-danger" style="margin-top: 5px;" onclick="uploadOta()"><i class="mdi mdi-upload"></i> Start Update</button>
                    </div>
                    <div class="progress-container" id="prg_container">
                        <div class="progress-bar" id="prg_bar"></div>
                    </div>
                    <div id="ota_status" style="margin-top: 10px; font-size: 0.95rem; text-align: center; color: var(--text-muted);"></div>
                </div>
            </div>

            <div class="panel">
                <div class="panel-section">
                    <h3><i class="mdi mdi-database-export-outline" style="color: var(--primary); margin-right: 6px;"></i> Backup & Restore Settings</h3>
                    <p style="color: var(--text-muted); font-size: 0.9rem; margin-bottom: 15px;">Save the current settings to a backup file, or upload a previously saved file to restore them.</p>
                    <div style="display: flex; gap: 10px; flex-wrap: wrap;">
                        <button class="btn" onclick="backupConfig()"><i class="mdi mdi-download"></i> Download Backup</button>
                        <button class="btn" onclick="document.getElementById('restoreFile').click()"><i class="mdi mdi-upload"></i> Restore from File</button>
                        <input type="file" id="restoreFile" style="display: none;" accept=".json" onchange="restoreConfig(event)">
                    </div>
                </div>
            </div>

            <div class="panel">
                <div class="panel-section">
                    <h3><i class="mdi mdi-restart" style="color: var(--primary); margin-right: 6px;"></i> Reboot Device</h3>
                    <p style="color: var(--text-muted); font-size: 0.9rem; margin-bottom: 15px;">Restart the weather station to reload configurations and re-initialize all sensors.</p>
                    <button class="btn" onclick="rebootDevice()"><i class="mdi mdi-restart"></i> Reboot Device</button>
                </div>
            </div>

            <div class="panel" style="border-color: rgba(239, 68, 68, 0.3);">
                <div class="panel-section">
                    <h3 style="color: var(--danger);"><i class="mdi mdi-alert-circle-outline" style="color: var(--danger); margin-right: 6px;"></i> Factory Reset</h3>
                    <p style="color: var(--text-muted); font-size: 0.9rem; margin-bottom: 15px;">Reset all saved configurations (Wi-Fi, MQTT, and calibration) and restart the ESP8266.</p>
                    <button class="btn btn-danger" onclick="factoryReset()"><i class="mdi mdi-alert"></i> Reset Device</button>
                </div>
            </div>
        </div>

        <!-- WIRING TAB -->
        <div id="info" class="tab-content">
            <div class="panel">
                <div class="panel-section">
                    <h3><i class="mdi mdi-transit-connection-variant" style="color: var(--primary); margin-right: 6px;"></i> Sensor Connection Diagram</h3>
                    <p style="color: var(--text-muted); font-size: 0.9rem; margin-bottom: 20px;">
                        This table dynamically updates based on the GPIO pins configured in your Settings.
                    </p>
                    <table class="info-table">
                        <thead>
                            <tr style="border-bottom: 2px solid var(--card-border); font-weight: bold; color: var(--primary);">
                                <th style="text-align: left; padding: 12px 8px;">Sensor / Function</th>
                                <th style="text-align: left; padding: 12px 8px;">NodeMCU Pin</th>
                                <th style="text-align: left; padding: 12px 8px;">ESP8266 GPIO</th>
                                <th style="text-align: left; padding: 12px 8px;">Description</th>
                            </tr>
                        </thead>
                        <tbody id="wiring_table_body">
                            <!-- Populated dynamically via JS -->
                        </tbody>
                    </table>
                </div>
            </div>

            <div class="panel" style="margin-top: 20px;">
                <div class="panel-section">
                    <h3><i class="mdi mdi-chip-outline" style="color: var(--primary); margin-right: 6px;"></i> I2C Sensor Status</h3>
                    <table class="info-table">
                        <tr><td style="font-weight: 600;">AHT20/AHT21 (Humidity/Temp)</td><td style="font-family: monospace;">0x38</td><td><span id="sns_aht">--</span></td></tr>
                        <tr><td style="font-weight: 600;">BMP280 (Pressure/Temp)</td><td style="font-family: monospace;">0x76/0x77</td><td><span id="sns_bmp">--</span></td></tr>
                        <tr><td style="font-weight: 600;">ENS160 (Air Quality)</td><td style="font-family: monospace;">0x52/0x53</td><td><span id="sns_ens">--</span></td></tr>
                        <tr><td style="font-weight: 600;">AS5600 (Wind Direction)</td><td style="font-family: monospace;">0x36</td><td><span id="sns_as5600">--</span></td></tr>
                        <tr><td style="font-weight: 600;">BH1750 (Luminosity)</td><td style="font-family: monospace;">0x23</td><td><span id="sns_bh1750">--</span></td></tr>
                    </table>
                </div>
            </div>

            <div class="panel" style="margin-top: 20px;">
                <div class="panel-section">
                    <h3><i class="mdi mdi-api" style="color: var(--primary); margin-right: 6px;"></i> REST API Reference</h3>
                    <p style="color: var(--text-muted); font-size: 0.9rem; margin-bottom: 20px;">
                        The weather station exposes a REST API that returns data in JSON format. You can integrate these endpoints into Home Assistant, Node-RED, or custom scripts.
                    </p>
                    
                    <div style="display: flex; flex-direction: column; gap: 15px;">
                        <!-- API 1: /api/status -->
                        <div style="border: 1px solid var(--card-border); border-radius: 8px; padding: 15px; background: rgba(0,0,0,0.15);">
                            <div style="display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 10px; margin-bottom: 8px;">
                                <span style="font-family: monospace; font-weight: bold; font-size: 1rem; color: var(--primary);">GET /api/status</span>
                                <span style="background: rgba(16, 185, 129, 0.15); color: var(--success); font-size: 0.8rem; font-weight: 600; padding: 3px 8px; border-radius: 4px; border: 1px solid rgba(16, 185, 129, 0.3);">Telemetry Data</span>
                            </div>
                            <p style="color: var(--text-muted); font-size: 0.85rem; margin-bottom: 8px;">Returns real-time sensor measurements, uptime, VCC, and device status flags.</p>
                            <details style="cursor: pointer;">
                                <summary style="color: var(--primary); font-size: 0.85rem; font-weight: 600; outline: none; margin-bottom: 5px;">Show Response Example</summary>
                                <pre style="font-family: 'JetBrains Mono', monospace; font-size: 0.8rem; background: rgba(0,0,0,0.3); padding: 10px; border-radius: 6px; color: var(--console-text); border: 1px solid var(--card-border); overflow-x: auto; margin-top: 5px;">{
  "tips": 0,
  "total_rain": 0.00,
  "hourly_rain": 0.00,
  "daily_rain": 0.00,
  "is_raining": false,
  "wind_speed": 0.0,
  "wind_gust": 0.0,
  "wind_dir": 311.0,
  "lux": 922.5,
  "temp": 30.5,
  "hum": 53.7,
  "press": 1018.9,
  "tvoc": 269,
  "eco2": 772,
  "aqi": 3,
  "vcc": 3.05,
  "uptime": "00:00:18",
  "ssid": "HOME-NET",
  "ip": "192.168.1.144"
}</pre>
                            </details>
                        </div>

                        <!-- API 2: /api/config -->
                        <div style="border: 1px solid var(--card-border); border-radius: 8px; padding: 15px; background: rgba(0,0,0,0.15);">
                            <div style="display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 10px; margin-bottom: 8px;">
                                <span style="font-family: monospace; font-weight: bold; font-size: 1rem; color: var(--primary);">GET /api/config</span>
                                <span style="background: rgba(245, 158, 11, 0.15); color: var(--warning); font-size: 0.8rem; font-weight: 600; padding: 3px 8px; border-radius: 4px; border: 1px solid rgba(245, 158, 11, 0.3);">Configuration Settings</span>
                            </div>
                            <p style="color: var(--text-muted); font-size: 0.85rem; margin-bottom: 8px;">Returns current settings (Wi-Fi, MQTT, offsets, calibration values).</p>
                            <details style="cursor: pointer;">
                                <summary style="color: var(--primary); font-size: 0.85rem; font-weight: 600; outline: none; margin-bottom: 5px;">Show Response Example</summary>
                                <pre style="font-family: 'JetBrains Mono', monospace; font-size: 0.8rem; background: rgba(0,0,0,0.3); padding: 10px; border-radius: 6px; color: var(--console-text); border: 1px solid var(--card-border); overflow-x: auto; margin-top: 5px;">{
  "hostname": "WeatherStation",
  "w_rad": 80,
  "w_mag": 1,
  "w_fac": 3.0,
  "w_cal": 5.4287,
  "dhcp": true,
  "ip": "192.168.1.144",
  "dns_p": "192.168.1.4",
  "ntp": "pool.ntp.org",
  "mqtt": "192.168.1.2",
  "mqtt_dec": 1
}</pre>
                            </details>
                        </div>

                        <!-- API 3: /api/clear_counters -->
                        <div style="border: 1px solid var(--card-border); border-radius: 8px; padding: 15px; background: rgba(0,0,0,0.15);">
                            <div style="display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 10px; margin-bottom: 8px;">
                                <span style="font-family: monospace; font-weight: bold; font-size: 1rem; color: var(--primary);">POST /api/clear_counters</span>
                                <span style="background: rgba(239, 68, 68, 0.15); color: var(--danger); font-size: 0.8rem; font-weight: 600; padding: 3px 8px; border-radius: 4px; border: 1px solid rgba(239, 68, 68, 0.3);">Reset Counters</span>
                            </div>
                            <p style="color: var(--text-muted); font-size: 0.85rem;">Resets the total rain tips counter back to 0.0 mm.</p>
                        </div>

                        <!-- API 4: /api/reboot -->
                        <div style="border: 1px solid var(--card-border); border-radius: 8px; padding: 15px; background: rgba(0,0,0,0.15);">
                            <div style="display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 10px; margin-bottom: 8px;">
                                <span style="font-family: monospace; font-weight: bold; font-size: 1rem; color: var(--primary);">POST /api/reboot</span>
                                <span style="background: rgba(239, 68, 68, 0.15); color: var(--danger); font-size: 0.8rem; font-weight: 600; padding: 3px 8px; border-radius: 4px; border: 1px solid rgba(239, 68, 68, 0.3);">Reboot Device</span>
                            </div>
                            <p style="color: var(--text-muted); font-size: 0.85rem;">Triggers a soft restart of the ESP8266 weather station.</p>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>
        let currentTab = 'dashboard';
        let statusInterval = null;
        let consoleInterval = null;

        function switchTab(tabId) {
            document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));
            
            currentTab = tabId;
            const activeBtn = Array.from(document.querySelectorAll('.tab-btn')).find(btn => btn.innerText.toLowerCase().includes(tabId.substring(0, 4)));
            if (activeBtn) activeBtn.classList.add('active');
            
            const activeContent = document.getElementById(tabId);
            if (activeContent) activeContent.classList.add('active');

            if (tabId === 'console') {
                loadConsoleLogs();
                if (!consoleInterval) {
                    consoleInterval = setInterval(loadConsoleLogs, 1500);
                }
            } else {
                if (consoleInterval) {
                    clearInterval(consoleInterval);
                    consoleInterval = null;
                }
            }
        }

        function getWindCardinal(deg) {
            const sectors = ["N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", 
                             "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"];
            const idx = Math.round(deg / 22.5) % 16;
            return sectors[idx];
        }

        function getAqiText(aqi) {
            switch(aqi) {
                case 1: return 'Excellent';
                case 2: return 'Good';
                case 3: return 'Moderate';
                case 4: return 'Poor';
                case 5: return 'Unhealthy';
                default: return 'Warming Up...';
            }
        }

        function getAqiLabel(aqi) {
            switch(aqi) {
                case 1: return '<span style="color: var(--success); font-weight: bold;">1 - Excellent</span>';
                case 2: return '<span style="color: #84cc16; font-weight: bold;">2 - Good</span>';
                case 3: return '<span style="color: #eab308; font-weight: bold;">3 - Moderate</span>';
                case 4: return '<span style="color: #f97316; font-weight: bold;">4 - Poor</span>';
                case 5: return '<span style="color: var(--danger); font-weight: bold;">5 - Unhealthy</span>';
                default: return '<span style="color: var(--primary); font-weight: bold;">Warming Up...</span>';
            }
        }

        function toggleUiMode(mode) {
            const classicGrid = document.getElementById('classic_dashboard_grid');
            const compactBoard = document.getElementById('compact_dashboard_board');
            if (classicGrid && compactBoard) {
                if (mode === 'compact') {
                    classicGrid.style.display = 'none';
                    compactBoard.style.display = 'block';
                } else {
                    classicGrid.style.display = 'block';
                    compactBoard.style.display = 'none';
                }
            }
        }

        function updateStatus() {
            fetch('/api/status')
                .then(res => res.json())
                .then(data => {
                    const isImp = !!data.use_imperial;
                    const rMult = isImp ? (1.0 / 25.4) : 1.0;
                    const rUnit = isImp ? 'in' : 'mm';

                    document.getElementById('val_hour').innerText = (data.hourly_rain * rMult).toFixed(2);
                    document.getElementById('val_day').innerText = (data.daily_rain * rMult).toFixed(2);
                    document.getElementById('val_total').innerText = (data.total_rain * rMult).toFixed(2);
                    if (document.getElementById('unit_hour')) document.getElementById('unit_hour').innerText = rUnit;
                    if (document.getElementById('unit_day')) document.getElementById('unit_day').innerText = rUnit;
                    if (document.getElementById('unit_total')) document.getElementById('unit_total').innerText = rUnit;

                    document.getElementById('sys_time').innerText = data.time || 'N/A';
                    document.getElementById('sys_tips').innerText = data.tips;
                    document.getElementById('sys_ssid').innerText = data.ssid || 'N/A';
                    document.getElementById('sys_rssi').innerText = data.rssi || 'N/A';
                    document.getElementById('sys_ip').innerText = data.ip;
                    document.getElementById('sys_uptime').innerText = data.uptime;
                    
                    const elVcc = document.getElementById('sys_vcc');
                    if (elVcc) {
                        elVcc.innerText = data.vcc ? (data.vcc.toFixed(2) + ' V') : 'N/A';
                    }

                    const elHeap = document.getElementById('sys_heap');
                    if (elHeap) {
                        elHeap.innerText = data.heap ? ((data.heap / 1024).toFixed(1) + ' KB') : 'N/A';
                    }

                    const setSnsStatus = (id, online) => {
                        const el = document.getElementById(id);
                        if (el) {
                            if (online) {
                                el.innerHTML = '<span style="color: var(--success); font-weight: bold;">Online</span>';
                            } else {
                                el.innerHTML = '<span style="color: var(--danger); font-weight: bold;">Offline</span>';
                            }
                        }
                    };
                    setSnsStatus('sns_aht', data.has_aht20);
                    setSnsStatus('sns_bmp', data.has_bmp280);
                    setSnsStatus('sns_ens', data.has_ens160);
                    setSnsStatus('sns_as5600', data.has_as5600);
                    setSnsStatus('sns_bh1750', data.has_bh1750);

                    if (data.ui_compact !== undefined) {
                        toggleUiMode(data.ui_compact ? 'compact' : 'classic');
                    }
                    
                    const valTemp = document.getElementById('val_temp');
                    if (data.has_aht20 || data.has_bmp280) {
                        const tVal = isImp ? (data.temp * 1.8 + 32.0) : data.temp;
                        const tUnit = isImp ? '°F' : '°C';
                        valTemp.innerHTML = `${tVal.toFixed(1)} <span class="card-unit">${tUnit}</span>`;
                    } else {
                        valTemp.innerHTML = '<span style="color: var(--danger); font-size: 1rem; font-weight: bold;">Sensor not found</span>';
                    }

                    const valHum = document.getElementById('val_hum');
                    if (data.has_aht20) {
                        valHum.innerHTML = `${data.hum.toFixed(1)} <span class="card-unit">%</span>`;
                    } else {
                        valHum.innerHTML = '<span style="color: var(--danger); font-size: 1rem; font-weight: bold;">Sensor not found</span>';
                    }

                    const valPress = document.getElementById('val_press');
                    if (data.has_bmp280) {
                        const pVal = isImp ? (data.press * 0.02953) : data.press;
                        const pUnit = isImp ? 'inHg' : 'hPa';
                        const pDec = isImp ? 2 : 1;
                        valPress.innerHTML = `${pVal.toFixed(pDec)} <span class="card-unit">${pUnit}</span>`;
                    } else {
                        valPress.innerHTML = '<span style="color: var(--danger); font-size: 1rem; font-weight: bold;">Sensor not found</span>';
                    }

                    const valAqi = document.getElementById('val_aqi');
                    const valEco2 = document.getElementById('val_eco2');
                    const valTvoc = document.getElementById('val_tvoc');
                    if (data.has_ens160) {
                        valAqi.innerHTML = `${data.aqi} <span class="card-unit">${getAqiLabel(data.aqi)}</span>`;
                        valEco2.innerHTML = `${data.eco2} <span class="card-unit">ppm</span>`;
                        valTvoc.innerHTML = `${data.tvoc} <span class="card-unit">ppb</span>`;
                    } else {
                        valAqi.innerHTML = '<span style="color: var(--danger); font-size: 1rem; font-weight: bold;">Sensor not found</span>';
                        valEco2.innerHTML = '<span style="color: var(--danger); font-size: 1rem; font-weight: bold;">Sensor not found</span>';
                        valTvoc.innerHTML = '<span style="color: var(--danger); font-size: 1rem; font-weight: bold;">Sensor not found</span>';
                    }

                    const wSpeed = isImp ? (data.wind_speed_mph || (data.wind_speed * 0.621371)) : data.wind_speed;
                    const wGust  = isImp ? (data.wind_gust_mph  || (data.wind_gust  * 0.621371)) : data.wind_gust;
                    const wUnit  = isImp ? 'mph' : 'km/h';

                    document.getElementById('val_wind_speed').innerHTML = `${wSpeed.toFixed(1)} <span class="card-unit">${wUnit}</span>`;
                    document.getElementById('val_wind_gust').innerHTML = `${wGust.toFixed(1)} <span class="card-unit">${wUnit}</span>`;

                    const subSpeed = document.getElementById('val_wind_speed_sub');
                    if (subSpeed) {
                        const ms = data.wind_speed_ms !== undefined ? data.wind_speed_ms : (data.wind_speed / 3.6);
                        const kt = data.wind_speed_kt !== undefined ? data.wind_speed_kt : (data.wind_speed * 0.539957);
                        subSpeed.innerHTML = `${ms.toFixed(1)} m/s &bull; ${kt.toFixed(1)} kt`;
                    }

                    const subGust = document.getElementById('val_wind_gust_sub');
                    if (subGust) {
                        const ms = data.wind_gust_ms !== undefined ? data.wind_gust_ms : (data.wind_gust / 3.6);
                        const kt = data.wind_gust_kt !== undefined ? data.wind_gust_kt : (data.wind_gust * 0.539957);
                        subGust.innerHTML = `${ms.toFixed(1)} m/s &bull; ${kt.toFixed(1)} kt`;
                    }

                    const valWindDir = document.getElementById('val_wind_dir');
                    if (data.has_as5600) {
                        valWindDir.innerHTML = `${data.wind_dir.toFixed(0)}&deg; <span class="card-unit" style="font-size: 1.2rem; font-weight: bold; margin-left: 5px;">${getWindCardinal(data.wind_dir)}</span>`;
                    } else {
                        valWindDir.innerHTML = '<span style="color: var(--danger); font-size: 1rem; font-weight: bold;">Sensor not found</span>';
                    }

                    const valLux = document.getElementById('val_lux');
                    if (data.has_bh1750) {
                        valLux.innerHTML = `${data.lux.toFixed(0)} <span class="card-unit">lx</span>`;
                    } else {
                        valLux.innerHTML = '<span style="color: var(--danger); font-size: 1rem; font-weight: bold;">Sensor not found</span>';
                    }

                    const badge = document.getElementById('rain_status_badge');
                    const badgeText = document.getElementById('rain_status_text');
                    
                    if (data.is_raining) {
                        badge.className = "status-badge badge-rain";
                        badgeText.innerText = "Raining";
                    } else {
                        badge.className = "status-badge badge-clear";
                        badgeText.innerText = "No Rain";
                    }

                    // Populate Compact Board elements
                    if (document.getElementById('compact_dashboard_board')) {
                        const cTemp = document.getElementById('c_val_temp');
                        if (cTemp) {
                            if (data.has_aht20 || data.has_bmp280) {
                                const tVal = isImp ? (data.temp * 1.8 + 32.0) : data.temp;
                                const tUnit = isImp ? '°F' : '°C';
                                cTemp.innerHTML = `${tVal.toFixed(1)} <span class="c-unit">${tUnit}</span>`;
                            } else {
                                cTemp.innerHTML = '<span style="color: var(--danger); font-size: 0.85rem;">N/A</span>';
                            }
                        }

                        const cHum = document.getElementById('c_val_hum');
                        if (cHum) {
                            if (data.has_aht20) {
                                cHum.innerHTML = `${data.hum.toFixed(1)} <span class="c-unit">%</span>`;
                            } else {
                                cHum.innerHTML = '<span style="color: var(--danger); font-size: 0.85rem;">N/A</span>';
                            }
                        }

                        const cPress = document.getElementById('c_val_press');
                        if (cPress) {
                            if (data.has_bmp280) {
                                const pVal = isImp ? (data.press * 0.02953) : data.press;
                                const pUnit = isImp ? 'inHg' : 'hPa';
                                const pDec = isImp ? 2 : 1;
                                cPress.innerHTML = `${pVal.toFixed(pDec)} <span class="c-unit">${pUnit}</span>`;
                            } else {
                                cPress.innerHTML = '<span style="color: var(--danger); font-size: 0.85rem;">N/A</span>';
                            }
                        }

                        const cLux = document.getElementById('c_val_lux');
                        if (cLux) {
                            if (data.has_bh1750) {
                                cLux.innerHTML = `${data.lux.toFixed(0)} <span class="c-unit">lx</span>`;
                            } else {
                                cLux.innerHTML = '<span style="color: var(--danger); font-size: 0.85rem;">N/A</span>';
                            }
                        }

                        const cWSpd = document.getElementById('c_val_wind_speed');
                        const cWGust = document.getElementById('c_val_wind_gust');
                        const cWSub = document.getElementById('c_val_wind_sub');
                        if (cWSpd && cWGust && cWSub) {
                            cWSpd.innerHTML = `${wSpeed.toFixed(1)} <span class="c-unit">${wUnit}</span>`;
                            cWGust.innerHTML = `(Gust: ${wGust.toFixed(1)})`;

                            const ms = data.wind_speed_ms !== undefined ? data.wind_speed_ms : (data.wind_speed / 3.6);
                            const kt = data.wind_speed_kt !== undefined ? data.wind_speed_kt : (data.wind_speed * 0.539957);
                            cWSub.innerHTML = `${ms.toFixed(1)} m/s &bull; ${kt.toFixed(1)} kt`;
                        }

                        const cWDir = document.getElementById('c_val_wind_dir');
                        if (cWDir) {
                            if (data.has_as5600) {
                                cWDir.innerHTML = `${data.wind_dir.toFixed(0)}&deg; <span class="c-unit" style="font-weight: bold;">${getWindCardinal(data.wind_dir)}</span>`;
                            } else {
                                cWDir.innerHTML = '<span style="color: var(--danger); font-size: 0.85rem;">N/A</span>';
                            }
                        }

                        const cRain1h = document.getElementById('c_val_rain_1h');
                        const cRain24h = document.getElementById('c_val_rain_24h');
                        const cRainTot = document.getElementById('c_val_rain_total');
                        const cRainUnit = document.getElementById('c_unit_rain');
                        if (cRain1h && cRain24h && cRainTot && cRainUnit) {
                            cRain1h.innerText = (data.hourly_rain * rMult).toFixed(2);
                            cRain24h.innerText = (data.daily_rain * rMult).toFixed(2);
                            cRainTot.innerText = (data.total_rain * rMult).toFixed(2);
                            cRainUnit.innerText = rUnit;
                        }

                        const cAqi = document.getElementById('c_val_aqi');
                        const cEco2 = document.getElementById('c_val_eco2');
                        const cTvoc = document.getElementById('c_val_tvoc');
                        if (cAqi && cEco2 && cTvoc) {
                            if (data.has_ens160) {
                                cAqi.innerHTML = `${data.aqi} <span style="font-size: 0.8rem; font-weight: bold;">(${getAqiText(data.aqi)})</span>`;
                                cEco2.innerText = data.eco2;
                                cTvoc.innerText = data.tvoc;
                            } else {
                                cAqi.innerText = '--';
                                cEco2.innerText = '--';
                                cTvoc.innerText = '--';
                            }
                        }

                        const cRainBadge = document.getElementById('c_rain_badge');
                        const cRainText = document.getElementById('c_rain_text');
                        if (cRainBadge && cRainText) {
                            if (data.is_raining) {
                                cRainBadge.className = "c-status-badge badge-rain";
                                cRainText.innerText = "Raining";
                            } else {
                                cRainBadge.className = "c-status-badge badge-clear";
                                cRainText.innerText = "No Rain";
                            }
                        }

                        if (document.getElementById('c_val_time')) document.getElementById('c_val_time').innerText = data.time ? data.time.substring(11, 19) : '--:--';
                        if (document.getElementById('c_val_ssid')) document.getElementById('c_val_ssid').innerText = data.ssid || 'WiFi';
                        if (document.getElementById('c_val_rssi')) document.getElementById('c_val_rssi').innerText = data.rssi || '';
                        if (document.getElementById('c_val_ip')) document.getElementById('c_val_ip').innerText = data.ip || '';
                        if (document.getElementById('c_val_uptime')) document.getElementById('c_val_uptime').innerText = data.uptime || '';
                    }
                })
                .catch(err => console.error("Error updating status:", err));
        }

        function clearCounters() {
            if (confirm("Are you sure you want to reset the total rain counter?")) {
                fetch('/api/clear_counters', { method: 'POST' }).then(() => updateStatus());
            }
        }

        function gpioToNodeMCU(gpio) {
            const mapping = {
                16: "D0",
                5: "D1",
                4: "D2",
                0: "D3",
                2: "D4",
                14: "D5",
                12: "D6",
                13: "D7",
                15: "D8",
                3: "RX",
                1: "TX"
            };
            return mapping[gpio] || `GPIO ${gpio}`;
        }

        function populateWiringTable(config) {
            const tbody = document.getElementById('wiring_table_body');
            if (!tbody) return;
            
            const sda = config.sda;
            const scl = config.scl;
            const rain = config.pin;
            const wind = config.w_pin;
            
            tbody.innerHTML = `
                <tr>
                    <td style="padding: 12px 8px; font-weight: 600;">I2C SDA (Data)</td>
                    <td style="padding: 12px 8px; font-weight: bold; color: var(--primary);">${gpioToNodeMCU(sda)}</td>
                    <td style="padding: 12px 8px; font-family: monospace;">GPIO ${sda}</td>
                    <td style="padding: 12px 8px; color: var(--text-muted);">Shared data line for BH1750, AHT20/21, BMP280, ENS160, AS5600</td>
                </tr>
                <tr>
                    <td style="padding: 12px 8px; font-weight: 600;">I2C SCL (Clock)</td>
                    <td style="padding: 12px 8px; font-weight: bold; color: var(--primary);">${gpioToNodeMCU(scl)}</td>
                    <td style="padding: 12px 8px; font-family: monospace;">GPIO ${scl}</td>
                    <td style="padding: 12px 8px; color: var(--text-muted);">Shared clock line for BH1750, AHT20/21, BMP280, ENS160, AS5600</td>
                </tr>
                <tr>
                    <td style="padding: 12px 8px; font-weight: 600;">Rain Gauge (Pluviometer)</td>
                    <td style="padding: 12px 8px; font-weight: bold; color: var(--warning);">${gpioToNodeMCU(rain)}</td>
                    <td style="padding: 12px 8px; font-family: monospace;">GPIO ${rain}</td>
                    <td style="padding: 12px 8px; color: var(--text-muted);">Hardware Interrupt for A3144 Hall-effect sensor</td>
                </tr>
                <tr>
                    <td style="padding: 12px 8px; font-weight: 600;">Anemometer (Wind Speed)</td>
                    <td style="padding: 12px 8px; font-weight: bold; color: var(--warning);">${gpioToNodeMCU(wind)}</td>
                    <td style="padding: 12px 8px; font-family: monospace;">GPIO ${wind}</td>
                    <td style="padding: 12px 8px; color: var(--text-muted);">Hardware Interrupt for counting cup rotations</td>
                </tr>
                <tr>
                    <td style="padding: 12px 8px; font-weight: 600;">Sensor Power (VCC)</td>
                    <td style="padding: 12px 8px; font-weight: bold; color: var(--success);">3V3</td>
                    <td style="padding: 12px 8px; font-family: monospace;">3.3V</td>
                    <td style="padding: 12px 8px; color: var(--text-muted);">Common 3.3V power line from NodeMCU regulator</td>
                </tr>
                <tr>
                    <td style="padding: 12px 8px; font-weight: 600;">Ground (GND)</td>
                    <td style="padding: 12px 8px; font-weight: bold; color: var(--success);">GND</td>
                    <td style="padding: 12px 8px; font-family: monospace;">GND</td>
                    <td style="padding: 12px 8px; color: var(--text-muted);">Common reference ground for all sensors</td>
                </tr>
            `;
        }

        function updateLiveWindCal() {
            const elRad = document.getElementById('conf_w_rad');
            const elMag = document.getElementById('conf_w_mag');
            const elFac = document.getElementById('conf_w_fac');
            const elCal = document.getElementById('conf_w_cal');
            
            if (elRad && elMag && elFac && elCal) {
                const radius = parseFloat(elRad.value) || 80;
                const magnets = parseInt(elMag.value) || 1;
                const factor = parseFloat(elFac.value) || 3.0;
                const cal = (7.2 * Math.PI * (radius / 1000.0) * factor) / magnets;
                elCal.value = cal.toFixed(4);
            }
        }

        function loadConfig() {
            fetch('/api/config')
                .then(res => res.json())
                .then(c => {
                    populateWiringTable(c);
                    document.getElementById('conf_host').value = c.hostname;
                    if (document.getElementById('conf_units')) {
                        document.getElementById('conf_units').value = c.use_imperial ? 'imperial' : 'metric';
                    }
                    if (document.getElementById('conf_ui_mode')) {
                        const mode = c.ui_compact ? 'compact' : 'classic';
                        document.getElementById('conf_ui_mode').value = mode;
                        toggleUiMode(mode);
                    }
                    if (document.getElementById('conf_tz')) {
                        document.getElementById('conf_tz').value = (c.tz_off !== undefined) ? c.tz_off : 1;
                    }
                    if (document.getElementById('conf_dst')) {
                        document.getElementById('conf_dst').value = (c.use_dst ? 'true' : 'false');
                    }
                    document.getElementById('conf_pin').value = c.pin;
                    document.getElementById('conf_cal').value = c.calibration;
                    document.getElementById('conf_deb').value = c.debounce;
                    document.getElementById('conf_dhcp').checked = c.dhcp;
                    document.getElementById('conf_crash_opt').checked = (c.crash_opt !== false);

                    document.getElementById('conf_sda').value = c.sda;
                    document.getElementById('conf_scl').value = c.scl;
                    document.getElementById('conf_i2c_clk').value = c.i2c_clk;
                    document.getElementById('conf_alt').value = c.altitude;
                    document.getElementById('conf_t_offset').value = c.t_offset;
                    document.getElementById('conf_h_offset').value = c.h_offset;
                    document.getElementById('conf_p_offset').value = c.p_offset;

                    document.getElementById('conf_w_pin').value = c.w_pin;
                    
                    // Safe loading of wind calibration parameters
                    if (document.getElementById('conf_w_rad')) {
                        document.getElementById('conf_w_rad').value = c.w_rad || 80;
                    }
                    if (document.getElementById('conf_w_mag')) {
                        document.getElementById('conf_w_mag').value = c.w_mag || 1;
                    }
                    if (document.getElementById('conf_w_fac')) {
                        document.getElementById('conf_w_fac').value = c.w_fac || 3.0;
                    }
                    if (typeof updateLiveWindCal === 'function') {
                        updateLiveWindCal();
                    }
                    
                    document.getElementById('conf_w_deb').value = c.w_deb;
                    document.getElementById('conf_w_dir_off').value = c.w_dir_off;
                    document.getElementById('conf_w_spd_int').value = c.w_spd_int || 2;
                    document.getElementById('conf_w_spd_avg').value = c.w_spd_avg || 5;
                    document.getElementById('conf_w_dir_avg').value = c.w_dir_avg || 5;
                    document.getElementById('conf_lux_cal').value = c.lux_cal;
                    document.getElementById('conf_mqtt_int').value = c.mqtt_int;
                    document.getElementById('conf_sens_int').value = c.sens_int;
                    document.getElementById('conf_mqtt_dec').value = c.mqtt_dec;
                    
                    document.getElementById('conf_ip').value = c.ip;
                    document.getElementById('conf_gw').value = c.gw;
                    document.getElementById('conf_nm').value = c.nm;
                    
                    // Safe loading of DNS and NTP
                    if (document.getElementById('conf_dns_p')) {
                        document.getElementById('conf_dns_p').value = c.dns_p || '';
                    }
                    if (document.getElementById('conf_dns_s')) {
                        document.getElementById('conf_dns_s').value = c.dns_s || '';
                    }
                    if (document.getElementById('conf_ntp')) {
                        document.getElementById('conf_ntp').value = c.ntp || '';
                    }
                    
                    document.getElementById('conf_mq').value = c.mqtt;
                    document.getElementById('conf_m_port').value = c.m_port;
                    document.getElementById('conf_m_user').value = c.m_user;
                    document.getElementById('conf_m_pass').value = c.m_pass;

                    toggleDhcpFields(c.dhcp);
                    document.getElementById('fw_ver_title').innerText = c.fw_version || '1.0.0';
                });
        }

        function toggleDhcpFields(checked) {
            document.getElementById('conf_ip').disabled = checked;
            document.getElementById('conf_gw').disabled = checked;
            document.getElementById('conf_nm').disabled = checked;
            if (document.getElementById('conf_dns_p')) {
                document.getElementById('conf_dns_p').disabled = checked;
            }
            if (document.getElementById('conf_dns_s')) {
                document.getElementById('conf_dns_s').disabled = checked;
            }
        }

        function saveConfig(event) {
            event.preventDefault();
            const config = {
                hostname: document.getElementById('conf_host').value,
                use_imperial: document.getElementById('conf_units') ? (document.getElementById('conf_units').value === 'imperial') : false,
                ui_compact: document.getElementById('conf_ui_mode') ? (document.getElementById('conf_ui_mode').value === 'compact') : false,
                tz_off: document.getElementById('conf_tz') ? parseInt(document.getElementById('conf_tz').value) : 1,
                use_dst: document.getElementById('conf_dst') ? (document.getElementById('conf_dst').value === 'true') : false,
                pin: parseInt(document.getElementById('conf_pin').value),
                calibration: parseFloat(document.getElementById('conf_cal').value),
                debounce: parseInt(document.getElementById('conf_deb').value),
                dhcp: document.getElementById('conf_dhcp').checked,
                crash_opt: document.getElementById('conf_crash_opt').checked,
                sda: parseInt(document.getElementById('conf_sda').value),
                scl: parseInt(document.getElementById('conf_scl').value),
                i2c_clk: parseInt(document.getElementById('conf_i2c_clk').value),
                altitude: parseInt(document.getElementById('conf_alt').value),
                t_offset: parseFloat(document.getElementById('conf_t_offset').value),
                h_offset: parseFloat(document.getElementById('conf_h_offset').value),
                p_offset: parseFloat(document.getElementById('conf_p_offset').value),
                w_pin: parseInt(document.getElementById('conf_w_pin').value),
                w_rad: document.getElementById('conf_w_rad') ? parseInt(document.getElementById('conf_w_rad').value) : 80,
                w_mag: document.getElementById('conf_w_mag') ? parseInt(document.getElementById('conf_w_mag').value) : 1,
                w_fac: document.getElementById('conf_w_fac') ? parseFloat(document.getElementById('conf_w_fac').value) : 3.0,
                w_deb: parseInt(document.getElementById('conf_w_deb').value),
                w_dir_off: parseInt(document.getElementById('conf_w_dir_off').value),
                w_spd_int: parseInt(document.getElementById('conf_w_spd_int').value),
                w_spd_avg: parseInt(document.getElementById('conf_w_spd_avg').value),
                w_dir_avg: parseInt(document.getElementById('conf_w_dir_avg').value),
                lux_cal: parseFloat(document.getElementById('conf_lux_cal').value),
                ip: document.getElementById('conf_ip').value,
                gw: document.getElementById('conf_gw').value,
                nm: document.getElementById('conf_nm').value,
                dns_p: document.getElementById('conf_dns_p') ? document.getElementById('conf_dns_p').value : '',
                dns_s: document.getElementById('conf_dns_s') ? document.getElementById('conf_dns_s').value : '',
                ntp: document.getElementById('conf_ntp') ? document.getElementById('conf_ntp').value : '',
                mqtt: document.getElementById('conf_mq').value,
                m_port: parseInt(document.getElementById('conf_m_port').value),
                m_user: document.getElementById('conf_m_user').value,
                m_pass: document.getElementById('conf_m_pass').value,
                mqtt_int: parseInt(document.getElementById('conf_mqtt_int').value),
                sens_int: parseInt(document.getElementById('conf_sens_int').value),
                mqtt_dec: parseInt(document.getElementById('conf_mqtt_dec').value)
            };

            fetch('/api/save_config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            })
            .then(res => {
                if (res.ok) {
                    alert("Configuration saved! The device will restart to apply changes.");
                    setTimeout(() => window.location.reload(), 3000);
                } else {
                    alert("Error saving configuration.");
                }
            });
        }

        function calibrateNorth() {
            if (confirm("Point the wind vane physically towards NORTH and hold it steady. Confirm calibration?")) {
                fetch('/api/calibrate_north', {
                    method: 'POST'
                })
                .then(res => {
                    if (res.ok) {
                        return res.json();
                    } else {
                        return res.json().then(err => { throw new Error(err.message || "Unknown error"); });
                    }
                })
                .then(data => {
                    if (data.status === "success") {
                        document.getElementById('conf_w_dir_off').value = data.offset;
                        alert("Calibration completed successfully! New offset set to: " + data.offset + "°");
                    } else {
                        alert("Error during calibration: " + data.message);
                    }
                })
                .catch(err => {
                    alert("Unable to calibrate North: " + err.message);
                });
            }
        }

        function readUnfiltered() {
            const statusLabel = document.getElementById('cal_unfiltered_status');
            const nextBtn = document.getElementById('btn_cal_filtered');
            statusLabel.innerText = "Reading...";
            
            fetch('/api/calibrate_lux_unfiltered', { method: 'POST' })
                .then(res => {
                    if (res.ok) return res.json();
                    return res.json().then(err => { throw new Error(err.message || "Failed reading"); });
                })
                .then(data => {
                    if (data.status === "success") {
                        statusLabel.innerText = `Reference: ${data.unfiltered_lux.toFixed(0)} lx`;
                        statusLabel.style.color = "var(--success)";
                        nextBtn.disabled = false;
                        document.getElementById('cal_filtered_status').innerText = "Ready for Step 2";
                        document.getElementById('cal_filtered_status').style.color = "var(--text-muted)";
                    } else {
                        statusLabel.innerText = "Error: " + data.message;
                        statusLabel.style.color = "var(--danger)";
                    }
                })
                .catch(err => {
                    statusLabel.innerText = "Error: " + err.message;
                    statusLabel.style.color = "var(--danger)";
                });
        }

        function readFiltered() {
            const statusLabel = document.getElementById('cal_filtered_status');
            statusLabel.innerText = "Calibrating...";
            
            fetch('/api/calibrate_lux_filtered', { method: 'POST' })
                .then(res => {
                    if (res.ok) return res.json();
                    return res.json().then(err => { throw new Error(err.message || "Failed calibration"); });
                })
                .then(data => {
                    if (data.status === "success") {
                        statusLabel.innerText = `Calibrated! Transmission: ${data.transmission_pct.toFixed(1)}%`;
                        statusLabel.style.color = "var(--success)";
                        document.getElementById('conf_lux_cal').value = data.factor.toFixed(4);
                        alert(`Calibration Successful!\nTransmission: ${data.transmission_pct.toFixed(1)}%\nFactor: ${data.factor.toFixed(4)}`);
                    } else {
                        statusLabel.innerText = "Error: " + data.message;
                        statusLabel.style.color = "var(--danger)";
                    }
                })
                .catch(err => {
                    statusLabel.innerText = "Error: " + err.message;
                    statusLabel.style.color = "var(--danger)";
                });
        }

        // Live Console log polling
        let lastLogSize = 0;
        function loadConsoleLogs() {
            fetch('/api/logs')
                .then(res => res.json())
                .then(logs => {
                    const consoleLog = document.getElementById('console_log');
                    if (logs && logs.length > 0) {
                        const content = logs.join('\n');
                        if (content.length !== lastLogSize) {
                            const shouldScroll = consoleLog.scrollTop + consoleLog.clientHeight >= consoleLog.scrollHeight - 50;
                            consoleLog.innerText = content;
                            lastLogSize = content.length;
                            if (shouldScroll) {
                                consoleLog.scrollTop = consoleLog.scrollHeight;
                            }
                        }
                    } else {
                        consoleLog.innerText = "Console is empty.";
                    }
                });
        }

        function sendConsoleCommand(event) {
            event.preventDefault();
            const input = document.getElementById('console_input');
            const cmd = input.value.trim();
            if (!cmd) return;

            input.value = '';
            
            // Append command to local log immediately
            const consoleLog = document.getElementById('console_log');
            consoleLog.innerText += `\n> ${cmd}\n`;
            consoleLog.scrollTop = consoleLog.scrollHeight;

            fetch(`/api/command?cmd=${encodeURIComponent(cmd)}`, { method: 'POST' })
                .then(res => res.text())
                .then(resText => {
                    // Update log buffer immediately
                    loadConsoleLogs();
                });
        }

        function clearConsoleLog() {
            if (confirm("Do you want to clear stored logs?")) {
                fetch('/api/command?cmd=clear_logs', { method: 'POST' }).then(() => {
                    document.getElementById('console_log').innerText = '';
                    lastLogSize = 0;
                });
            }
        }

        function uploadOta() {
            const fileInput = document.getElementById('ota_file');
            const file = fileInput.files[0];
            if (!file) {
                alert("Please select a .bin file first!");
                return;
            }

            const formData = new FormData();
            formData.append("update", file);

            const prgContainer = document.getElementById('prg_container');
            const prgBar = document.getElementById('prg_bar');
            const otaStatus = document.getElementById('ota_status');

            prgContainer.style.display = 'block';
            otaStatus.innerText = "Uploading...";

            const xhr = new XMLHttpRequest();
            xhr.open("POST", "/api/ota");
            
            xhr.upload.addEventListener("progress", (e) => {
                if (e.lengthComputable) {
                    const pct = (e.loaded / e.total) * 100;
                    prgBar.style.style.width = pct + "%";
                    otaStatus.innerText = `Uploading: ${pct.toFixed(0)}%`;
                }
            });

            xhr.onload = function() {
                if (xhr.status == 200) {
                    otaStatus.innerHTML = "<span style='color: var(--success); font-weight: bold;'>Update Successful! Rebooting...</span>";
                    setTimeout(() => window.location.reload(), 5000);
                } else {
                    otaStatus.innerHTML = "<span style='color: var(--danger); font-weight: bold;'>Update Failed!</span>";
                }
            };

            xhr.send(formData);
        }

        function backupConfig() {
            fetch('/api/backup')
                .then(res => res.json())
                .then(data => {
                    // Remove runtime-only fw_version from backup file
                    delete data.fw_version;

                    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
                    const url = URL.createObjectURL(blob);
                    const a = document.createElement('a');
                    a.href = url;
                    a.download = `${data.hostname || 'meteo'}_config_${new Date().toISOString().slice(0,10)}.json`;
                    document.body.appendChild(a);
                    a.click();
                    document.body.removeChild(a);
                    URL.revokeObjectURL(url);
                })
                .catch(err => alert("Error downloading backup: " + err));
        }

        function restoreConfig(event) {
            const file = event.target.files[0];
            if (!file) return;

            const reader = new FileReader();
            reader.onload = function(e) {
                try {
                    const config = JSON.parse(e.target.result);
                    if (confirm("Restore configuration from file? This will overwrite your current settings and reboot.")) {
                        fetch('/api/save_config', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify(config)
                        })
                        .then(res => {
                            // /api/save_config returns plain text "OK" on success
                            if (res.ok) {
                                alert("Settings restored successfully! The device will reboot now to apply changes.");
                                setTimeout(() => window.location.reload(), 8000);
                            } else {
                                return res.text().then(t => { throw new Error(t); });
                            }
                        })
                        .catch(err => alert("Error restoring configuration: " + err));
                    }
                } catch (err) {
                    alert("Invalid JSON file: " + err.message);
                }
            };
            reader.readAsText(file);
            event.target.value = '';
        }

        function rebootDevice() {
            if (confirm("Are you sure you want to reboot the weather station?")) {
                fetch('/api/reboot', { method: 'POST' })
                    .then(res => {
                        alert("Rebooting weather station... Page will reload in 5 seconds.");
                        setTimeout(() => window.location.reload(), 5000);
                    });
            }
        }

        function factoryReset() {
            if (confirm("WARNING: This action will reset ALL settings and Wi-Fi credentials. Do you want to proceed?")) {
                fetch('/api/factory_reset', { method: 'POST' })
                    .then(res => {
                        alert("Device reset! The ESP8266 will restart in Access Point mode for configuration.");
                        setTimeout(() => window.location.reload(), 2000);
                    });
            }
        }

        // Init
        document.addEventListener('DOMContentLoaded', () => {
            updateStatus();
            loadConfig();
            statusInterval = setInterval(updateStatus, 3000);
        });
    </script>
</body>
</html>
)rawliteral";

// Implementation of logging to the ring buffer
char log_buffer[LOG_BUFFER_SIZE][LOG_LINE_LENGTH];
int log_buffer_head = 0;
int log_buffer_count = 0;

void app_log(const char* format, ...) {
    char line[LOG_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(line, sizeof(line), format, args);
    va_end(args);

    // Strip trailing newlines or carriage returns
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }

    // Print to Hardware Serial
    Serial.println(line);

    // Format with NTP time if synchronized, otherwise fall back to uptime millis
    char time_line[LOG_LINE_LENGTH + 32];
    time_t now = time(nullptr);
    struct tm* timeinfo = gmtime(&now);
    if (timeinfo->tm_year >= 120) {
        char time_buf[24];
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", timeinfo);
        snprintf(time_line, sizeof(time_line), "[%s GMT] %s", time_buf, line);
    } else {
        uint32_t ms = millis();
        snprintf(time_line, sizeof(time_line), "[%08u] %s", ms, line);
    }

    // Add to buffer
    strncpy(log_buffer[log_buffer_head], time_line, LOG_LINE_LENGTH - 1);
    log_buffer[log_buffer_head][LOG_LINE_LENGTH - 1] = '\0';
    
    log_buffer_head = (log_buffer_head + 1) % LOG_BUFFER_SIZE;
    if (log_buffer_count < LOG_BUFFER_SIZE) {
        log_buffer_count++;
    }
}

void clear_logs() {
    log_buffer_head = 0;
    log_buffer_count = 0;
    memset(log_buffer, 0, sizeof(log_buffer));
}

// Function to process console commands
String execute_console_command(String cmd) {
    cmd.trim();
    if (cmd == "") return "";

    app_log("Cmd: %s", cmd.c_str());

    int spaceIdx = cmd.indexOf(' ');
    String baseCmd = (spaceIdx == -1) ? cmd : cmd.substring(0, spaceIdx);
    String arg = (spaceIdx == -1) ? "" : cmd.substring(spaceIdx + 1);
    baseCmd.toLowerCase();

    if (baseCmd == "help") {
        app_log("Available commands:");
        app_log(" - help, status, reset, clear_logs, i2c_scan");
        app_log(" - clear_rain, set_cal <val>, set_deb <val>, set_pin <val>");
        app_log(" - clear_gust, calibrate_north, reset_ens160, ens160_diag, loglevel <info|debug>");
        return "OK";
    } 
    else if (baseCmd == "loglevel") {
        arg.toLowerCase();
        if (arg == "debug") {
            debug_logs_enabled = true;
            app_log("Loglevel set to DEBUG. I2C traffic logging enabled.");
        } else {
            debug_logs_enabled = false;
            app_log("Loglevel set to INFO. I2C traffic logging disabled.");
        }
        return "OK";
    }
    else if (baseCmd == "i2c_scan") {
        app_log("Scanning I2C bus...");
        int count = 0;
        for (byte address = 1; address < 127; address++) {
            Wire.beginTransmission(address);
            byte error = Wire.endTransmission();
            if (error == 0) {
                String desc = "Unknown Device";
                if (address == 0x23) desc = "BH1750 (Ambient Light Sensor/Lux)";
                else if (address == 0x36) desc = "AS5600 (Magnetic Rotary Encoder/Wind Direction)";
                else if (address == 0x38) desc = "AHT20/AHT21 (Temperature & Humidity Sensor)";
                else if (address == 0x52) desc = "ENS160 (Air Quality/eCO2/TVOC) [Alt Address]";
                else if (address == 0x53) desc = "ENS160 (Air Quality/eCO2/TVOC) [Standard Address]";
                else if (address == 0x76) desc = "BMP280 (Pressure & Temperature Sensor) [Alt Address]";
                else if (address == 0x77) desc = "BMP280 (Pressure & Temperature Sensor) [Standard Address]";
                
                app_log(" - 0x%02X : %s", address, desc.c_str());
                count++;
            }
        }
        if (count == 0) {
            app_log("No I2C devices found!");
        } else {
            app_log("Found %d devices.", count);
        }
        return "OK";
    }
    else if (baseCmd == "status") {
        String temp_str = (has_aht20 || has_bmp280) ? String(temperature_c, 1) + " C" : "N/A";
        String hum_str = has_aht20 ? String(humidity_pct, 1) + " %" : "N/A";
        String press_str = has_bmp280 ? String(pressure_hpa, 1) + " hPa" : "N/A";
        
        String aq_str = "N/A";
        if (has_ens160) {
            aq_str = "TVOC: " + String(ens160_tvoc) + " ppb, eCO2: " + String(ens160_eco2) + " ppm, AQI: " + String(ens160_aqi);
        }
        
        String dir_str = has_as5600 ? String(wind_dir_deg, 1) + " deg" : "N/A";
        String lux_str = has_bh1750 ? String(lux, 1) + " lx" : "N/A";
        
        app_log("Uptime: %lu s | Temp: %s | Hum: %s | Press: %s", millis() / 1000, temp_str.c_str(), hum_str.c_str(), press_str.c_str());
        app_log("Air: %s", aq_str.c_str());
        app_log("Wind: %s km/h (Gust: %s, Dir: %s) | Lux: %s", String(wind_speed_kmh, 1).c_str(), String(wind_gust_kmh, 1).c_str(), dir_str.c_str(), lux_str.c_str());
        app_log("Rain: %s mm (1h: %s, 24h: %s) | Tips: %u", String(total_rain_mm, 2).c_str(), String(rolling_rain_hour, 2).c_str(), String(rolling_rain_day, 2).c_str(), total_bucket_tips);
        return "OK";
    } 
    else if (baseCmd == "reset") {
        app_log("Reboot requested via console...");
        delay(500);
        request_reboot();
        return "Rebooting...";
    } 
    else if (baseCmd == "clear_rain") {
        total_bucket_tips = 0;
        last_processed_tips = 0;
        total_rain_mm = 0.0;
        memset(rain_history, 0, sizeof(rain_history));
        rolling_rain_hour = 0.0;
        rolling_rain_day = 0.0;
        
        prefs.begin("weather", false);
        prefs.putLong("tips", 0);
        prefs.end();
        
        app_log("Rain counters cleared.");
        return "Rain counters cleared.";
    }
    else if (baseCmd == "clear_gust") {
        reset_wind_gust();
        return "Wind gust counter cleared.";
    } 
    else if (baseCmd == "calibrate_north") {
        if (!has_as5600) {
            return "Error: AS5600 sensor not found.";
        }
        uint16_t raw = read_as5600_raw_angle();
        if (raw == 0xFFFF) {
            return "Error: failed to read raw angle.";
        }
        int new_offset = (int)(raw * 360.0f / 4096.0f + 0.5f) % 360;
        wind_dir_offset = new_offset;
        prefs.begin("weather", false);
        prefs.putInt("w_dir_off", wind_dir_offset);
        prefs.end();
        app_log("Console calibration: North offset set to %d deg.", wind_dir_offset);
        return "Calibration successful. Offset set to " + String(wind_dir_offset) + " degrees.";
    } 
    else if (baseCmd == "clear_logs") {
        clear_logs();
        return "Logs cleared.";
    }
    else if (baseCmd == "reset_ens160") {
        if (!has_ens160) {
            return "Error: ENS160 gas sensor not detected.";
        }
        reset_ens160_baseline();
        return "ENS160 baseline reset initiated successfully.";
    }
    else if (baseCmd == "ens160_diag") {
        if (!has_ens160) {
            return "Error: ENS160 gas sensor not detected.";
        }
        uint32_t rs0 = 0, rs1 = 0, rs2 = 0, rs3 = 0;
        get_ens160_resistances(rs0, rs1, rs2, rs3);
        char diagBuf[128];
        snprintf(diagBuf, sizeof(diagBuf), "ENS160 Hotplate Resistances: RS0: %u Ohm, RS1: %u Ohm, RS2: %u Ohm, RS3: %u Ohm",
                 rs0, rs1, rs2, rs3);
        app_log("%s", diagBuf);
        return String(diagBuf);
    }
    else if (baseCmd == "set_cal") {
        if (arg == "") return "Error: enter a decimal value.";
        float val = arg.toFloat();
        if (val <= 0.0) return "Error: invalid value.";
        
        rain_calibration = val;
        prefs.begin("weather", false);
        prefs.putFloat("cal", rain_calibration);
        prefs.end();
        
        app_log("Calibration set to %s mm/tip", String(rain_calibration, 4).c_str());
        return "Calibration updated.";
    } 
    else if (baseCmd == "set_deb") {
        if (arg == "") return "Error: enter milliseconds.";
        int val = arg.toInt();
        if (val < 0) return "Error: invalid value.";
        
        rain_debounce_ms = val;
        prefs.begin("weather", false);
        prefs.putInt("debounce", rain_debounce_ms);
        prefs.end();
        
        app_log("Debounce set to %u ms", rain_debounce_ms);
        return "Debounce updated.";
    } 
    else if (baseCmd == "set_pin") {
        if (arg == "") return "Error: enter GPIO pin.";
        int val = arg.toInt();
        if (val < 0 || val > 39) return "Error: invalid GPIO.";
        
        rain_sensor_pin = val;
        prefs.begin("weather", false);
        prefs.putInt("pin", rain_sensor_pin);
        prefs.end();
        
        app_log("Sensor pin set to GPIO %d. Reboot to apply.", rain_sensor_pin);
        return "Sensor pin updated. Reboot to apply (type 'reset').";
    }

    return "Unknown command. Type 'help' for the list.";
}

String get_formatted_time() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    if (timeinfo == nullptr || timeinfo->tm_year < 120) {
        return "NTP Syncing...";
    }
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buf);
}

void setup_web_server() {
    server.on("/", HTTP_GET, []() {
        server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "0");
        server.send_P(200, "text/html", index_html);
    });

    server.on("/api/status", HTTP_GET, []() {
        JsonDocument doc;
        doc["use_imperial"] = use_imperial;
        doc["ui_compact"] = ui_compact;
        doc["tips"] = total_bucket_tips;
        doc["total_rain"] = total_rain_mm;
        doc["hourly_rain"] = rolling_rain_hour;
        doc["daily_rain"] = rolling_rain_day;
        doc["is_raining"] = is_raining;
        
        doc["wind_speed"] = wind_speed_kmh;
        doc["wind_gust"] = wind_gust_kmh;
        doc["wind_speed_ms"] = wind_speed_kmh / 3.6f;
        doc["wind_speed_kt"] = wind_speed_kmh * 0.539957f;
        doc["wind_speed_mph"] = wind_speed_kmh * 0.621371f;
        doc["wind_gust_ms"] = wind_gust_kmh / 3.6f;
        doc["wind_gust_kt"] = wind_gust_kmh * 0.539957f;
        doc["wind_gust_mph"] = wind_gust_kmh * 0.621371f;
        
        doc["has_aht20"] = has_aht20;
        doc["has_bmp280"] = has_bmp280;
        doc["has_ens160"] = has_ens160;
        doc["has_as5600"] = has_as5600;
        doc["has_bh1750"] = has_bh1750;

        if (has_bh1750) {
            doc["lux"] = lux;
        } else {
            doc["lux"] = nullptr;
        }

        if (has_as5600) {
            doc["wind_dir"] = wind_dir_deg;
        } else {
            doc["wind_dir"] = nullptr;
        }
        
        if (has_aht20 || has_bmp280) {
            doc["temp"] = temperature_c;
        } else {
            doc["temp"] = nullptr;
        }
        
        if (has_aht20) {
            doc["hum"] = humidity_pct;
        } else {
            doc["hum"] = nullptr;
        }
        
        if (has_bmp280) {
            doc["press"] = pressure_hpa;
        } else {
            doc["press"] = nullptr;
        }

        if (has_ens160) {
            doc["tvoc"] = ens160_tvoc;
            doc["eco2"] = ens160_eco2;
            doc["aqi"] = ens160_aqi;
        } else {
            doc["tvoc"] = nullptr;
            doc["eco2"] = nullptr;
            doc["aqi"] = nullptr;
        }
        
        uint32_t up = millis() / 1000;
        int d = up / 86400; up %= 86400;
        int h = up / 3600; up %= 3600;
        int m = up / 60; int s = up % 60;
        char up_buf[32];
        if(d > 0) sprintf(up_buf, "%dd, %02d:%02d:%02d", d, h, m, s);
        else sprintf(up_buf, "%02d:%02d:%02d", h, m, s);
        
        doc["uptime"] = up_buf;
        doc["ssid"] = WiFi.SSID();
        doc["time"] = get_formatted_time();
        doc["rssi"] = String(WiFi.RSSI()) + " dBm";
        doc["ip"] = WiFi.localIP().toString();
        doc["vcc"] = ESP.getVcc() / 1000.0f;
        doc["heap"] = ESP.getFreeHeap();

        String res; serializeJson(doc, res);
        server.send(200, "application/json", res);
    });

    server.on("/api/config", HTTP_GET, []() {
        JsonDocument doc;
        doc["fw_version"] = FIRMWARE_VERSION;
        doc["hostname"] = hostname;
        doc["use_imperial"] = use_imperial;
        doc["ui_compact"] = ui_compact;
        doc["tz_off"] = timezone_offset_h;
        doc["use_dst"] = use_dst;
        doc["pin"] = rain_sensor_pin;
        doc["calibration"] = rain_calibration;
        doc["debounce"] = rain_debounce_ms;
        doc["dhcp"] = use_dhcp;
        doc["crash_opt"] = opt_in_crash_dump;
        
        doc["sda"] = i2c_sda_pin;
        doc["scl"] = i2c_scl_pin;
        doc["i2c_clk"] = i2c_clock_khz;
        doc["altitude"] = altitude_m;
        doc["t_offset"] = temp_offset;
        doc["h_offset"] = hum_offset;
        doc["p_offset"] = press_offset;

        doc["w_pin"] = wind_sensor_pin;
        doc["w_rad"] = wind_radius_mm;
        doc["w_mag"] = wind_magnets;
        doc["w_fac"] = wind_factor;
        doc["w_cal"] = wind_calibration;
        doc["w_deb"] = wind_debounce_ms;
        doc["w_dir_off"] = wind_dir_offset;
        doc["w_spd_int"] = wind_speed_interval_s;
        doc["w_spd_avg"] = wind_speed_avg_samples;
        doc["w_dir_avg"] = wind_dir_avg_samples;
        doc["lux_cal"] = lux_cal_factor;
        doc["mqtt_int"] = mqtt_publish_interval_s;
        doc["sens_int"] = sensor_read_interval_s;
        doc["mqtt_dec"] = mqtt_decimals;

        prefs.begin("weather", true);
        if(use_dhcp) {
            doc["ip"] = WiFi.localIP().toString();
            doc["gw"] = WiFi.gatewayIP().toString();
            doc["nm"] = WiFi.subnetMask().toString();
            doc["dns_p"] = WiFi.dnsIP().toString();
            doc["dns_s"] = WiFi.dnsIP(1).toString();
        } else {
            doc["ip"] = prefs.getString("ip", "192.168.1.150");
            doc["gw"] = prefs.getString("gw", "192.168.1.1");
            doc["nm"] = prefs.getString("nm", "255.255.255.0");
            doc["dns_p"] = prefs.getString("dns_p", "8.8.8.8");
            doc["dns_s"] = prefs.getString("dns_s", "8.8.4.4");
        }
        doc["ntp"] = prefs.getString("ntp", "pool.ntp.org");
        
        doc["mqtt"] = prefs.getString("mqtt_host", "");
        doc["m_port"] = prefs.getInt("mqtt_port", 1883);
        doc["m_user"] = prefs.getString("mqtt_user", "");
        doc["m_pass"] = prefs.getString("mqtt_pass", "");
        prefs.end();

        String res; serializeJson(doc, res);
        server.send(200, "application/json", res);
    });

    // Full backup endpoint — includes WiFi credentials from NVS for complete restore
    server.on("/api/backup", HTTP_GET, []() {
        JsonDocument doc;
        doc["fw_version"] = FIRMWARE_VERSION;
        doc["hostname"] = hostname;
        doc["use_imperial"] = use_imperial;
        doc["ui_compact"] = ui_compact;
        doc["tz_off"] = timezone_offset_h;
        doc["use_dst"] = use_dst;
        doc["pin"] = rain_sensor_pin;
        doc["calibration"] = rain_calibration;
        doc["debounce"] = rain_debounce_ms;
        doc["dhcp"] = use_dhcp;
        doc["crash_opt"] = opt_in_crash_dump;

        doc["sda"] = i2c_sda_pin;
        doc["scl"] = i2c_scl_pin;
        doc["i2c_clk"] = i2c_clock_khz;
        doc["altitude"] = altitude_m;
        doc["t_offset"] = temp_offset;
        doc["h_offset"] = hum_offset;
        doc["p_offset"] = press_offset;

        doc["w_pin"] = wind_sensor_pin;
        doc["w_rad"] = wind_radius_mm;
        doc["w_mag"] = wind_magnets;
        doc["w_fac"] = wind_factor;
        doc["w_cal"] = wind_calibration;
        doc["w_deb"] = wind_debounce_ms;
        doc["w_dir_off"] = wind_dir_offset;
        doc["w_spd_int"] = wind_speed_interval_s;
        doc["w_spd_avg"] = wind_speed_avg_samples;
        doc["w_dir_avg"] = wind_dir_avg_samples;
        doc["lux_cal"] = lux_cal_factor;
        doc["mqtt_int"] = mqtt_publish_interval_s;
        doc["sens_int"] = sensor_read_interval_s;
        doc["mqtt_dec"] = mqtt_decimals;

        Preferences local_prefs;
        local_prefs.begin("weather", true);

        // WiFi credentials (needed for full restore)
        doc["wifi_ssid"] = local_prefs.getString("wifi_ssid", "");
        doc["wifi_pass"] = local_prefs.getString("wifi_pass", "");

        // IP config — always from NVS (static values)
        doc["ip"]    = local_prefs.getString("ip",    wifi_ip.length()  ? wifi_ip  : WiFi.localIP().toString());
        doc["gw"]    = local_prefs.getString("gw",    wifi_gw.length()  ? wifi_gw  : WiFi.gatewayIP().toString());
        doc["nm"]    = local_prefs.getString("nm",    wifi_nm.length()  ? wifi_nm  : WiFi.subnetMask().toString());
        doc["dns_p"] = local_prefs.getString("dns_p", dns_primary.length()   ? dns_primary   : "8.8.8.8");
        doc["dns_s"] = local_prefs.getString("dns_s", dns_secondary.length() ? dns_secondary : "8.8.4.4");
        doc["ntp"]   = local_prefs.getString("ntp",   "pool.ntp.org");

        doc["mqtt"]   = local_prefs.getString("mqtt_host", "");
        doc["m_port"] = local_prefs.getInt("mqtt_port", 1883);
        doc["m_user"] = local_prefs.getString("mqtt_user", "");
        doc["m_pass"] = local_prefs.getString("mqtt_pass", "");
        local_prefs.end();

        String res; serializeJson(doc, res);
        server.send(200, "application/json", res);
    });

    server.on("/api/save_config", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                // Use a local Preferences instance to avoid conflicts with the global prefs object
                Preferences local_prefs;
                local_prefs.begin("weather", false);
                
                String new_host = doc["hostname"] | "WeatherStation";
                local_prefs.putString("hostname", new_host);
                hostname = new_host;

                if (doc["use_imperial"].is<bool>()) {
                    use_imperial = doc["use_imperial"].as<bool>();
                    local_prefs.putBool("use_imp", use_imperial);
                }

                if (doc["ui_compact"].is<bool>()) {
                    ui_compact = doc["ui_compact"].as<bool>();
                    local_prefs.putBool("ui_comp", ui_compact);
                }

                rain_sensor_pin = doc["pin"] | 14;
                rain_calibration = doc["calibration"] | 0.6314f;
                rain_debounce_ms = doc["debounce"] | 300;
                
                local_prefs.putInt("pin", rain_sensor_pin);
                local_prefs.putFloat("cal", rain_calibration);
                local_prefs.putInt("debounce", rain_debounce_ms);

                i2c_sda_pin = doc["sda"] | 4;
                i2c_scl_pin = doc["scl"] | 5;
                i2c_clock_khz = doc["i2c_clk"] | 100;
                altitude_m = doc["altitude"] | 0;
                temp_offset = doc["t_offset"] | 0.0f;
                hum_offset = doc["h_offset"] | 0.0f;
                press_offset = doc["p_offset"] | 0.0f;

                local_prefs.putInt("sda", i2c_sda_pin);
                local_prefs.putInt("scl", i2c_scl_pin);
                local_prefs.putInt("i2c_clk", i2c_clock_khz);
                local_prefs.putInt("alt", altitude_m);
                local_prefs.putFloat("t_offset", temp_offset);
                local_prefs.putFloat("h_offset", hum_offset);
                local_prefs.putFloat("p_offset", press_offset);

                wind_sensor_pin = doc["w_pin"] | 12;
                wind_radius_mm = doc["w_rad"] | 80;
                wind_magnets = doc["w_mag"] | 1;
                wind_factor = doc["w_fac"] | 3.0f;
                wind_debounce_ms = doc["w_deb"] | 15;

                local_prefs.putInt("w_pin", wind_sensor_pin);
                local_prefs.putInt("w_rad", wind_radius_mm);
                local_prefs.putInt("w_mag", wind_magnets);
                local_prefs.putFloat("w_fac", wind_factor);
                
                wind_calibration = (7.2f * 3.14159265f * (wind_radius_mm / 1000.0f) * wind_factor) / wind_magnets;
                local_prefs.putFloat("w_cal", wind_calibration);
                local_prefs.putInt("w_deb", wind_debounce_ms);

                wind_dir_offset = doc["w_dir_off"] | 0;
                local_prefs.putInt("w_dir_off", wind_dir_offset);

                wind_speed_avg_samples = max(1, min((int)(doc["w_spd_avg"] | 5), WIND_AVG_MAX_SAMPLES));
                wind_dir_avg_samples   = max(1, min((int)(doc["w_dir_avg"] | 5), WIND_AVG_MAX_SAMPLES));
                wind_speed_interval_s  = max(1, min((int)(doc["w_spd_int"] | 2), 60));
                local_prefs.putInt("w_spd_avg", wind_speed_avg_samples);
                local_prefs.putInt("w_dir_avg",  wind_dir_avg_samples);
                local_prefs.putInt("w_spd_int",  wind_speed_interval_s);
                // Reset ring buffers so new N takes effect immediately
                wind_speed_buf_idx = 0; wind_speed_buf_count = 0;
                wind_dir_buf_idx   = 0; wind_dir_buf_count   = 0;

                lux_cal_factor = doc["lux_cal"] | 1.0f;
                local_prefs.putFloat("lux_cal", lux_cal_factor);

                use_dhcp = doc["dhcp"].as<bool>();
                local_prefs.putBool("dhcp", use_dhcp);
                
                if (doc["crash_opt"].is<bool>()) {
                    opt_in_crash_dump = doc["crash_opt"].as<bool>();
                    local_prefs.putBool("crash_opt", opt_in_crash_dump);
                }
                
                if (!use_dhcp) {
                    String new_ip = doc["ip"] | "";
                    String new_gw = doc["gw"] | "";
                    String new_nm = doc["nm"] | "";
                    String dp = doc["dns_p"] | "8.8.8.8";
                    String ds = doc["dns_s"] | "8.8.4.4";
                    local_prefs.putString("ip", new_ip);
                    local_prefs.putString("gw", new_gw);
                    local_prefs.putString("nm", new_nm);
                    local_prefs.putString("dns_p", dp);
                    local_prefs.putString("dns_s", ds);
                    // Update global variables so next boot reads correct values
                    wifi_ip = new_ip;
                    wifi_gw = new_gw;
                    wifi_nm = new_nm;
                    dns_primary = dp;
                    dns_secondary = ds;
                }

                // Save WiFi credentials only if present in JSON (e.g. from a full backup restore)
                // This avoids overwriting saved credentials when saving from the UI settings page
                if (doc["wifi_ssid"].is<const char*>() && strlen(doc["wifi_ssid"] | "") > 0) {
                    String new_ssid = doc["wifi_ssid"].as<String>();
                    local_prefs.putString("wifi_ssid", new_ssid);
                    wifi_ssid = new_ssid;
                }
                if (doc["wifi_pass"].is<const char*>() && strlen(doc["wifi_pass"] | "") > 0) {
                    String new_pass = doc["wifi_pass"].as<String>();
                    local_prefs.putString("wifi_pass", new_pass);
                    wifi_pass = new_pass;
                }

                String ntp_srv = doc["ntp"] | "pool.ntp.org";
                local_prefs.putString("ntp", ntp_srv);
                ntp_server = ntp_srv;

                if (doc["tz_off"].is<int>()) {
                    timezone_offset_h = doc["tz_off"].as<int>();
                    local_prefs.putInt("tz_off", timezone_offset_h);
                }
                if (doc["use_dst"].is<bool>()) {
                    use_dst = doc["use_dst"].as<bool>();
                    local_prefs.putBool("use_dst", use_dst);
                }
                apply_time_zone_config();
                
                mqtt_publish_interval_s = doc["mqtt_int"] | 15;
                sensor_read_interval_s = doc["sens_int"] | 5;
                mqtt_decimals = doc["mqtt_dec"] | 1;
                local_prefs.putInt("mqtt_int", mqtt_publish_interval_s);
                local_prefs.putInt("sens_int", sensor_read_interval_s);
                local_prefs.putInt("mqtt_dec", mqtt_decimals);

                local_prefs.putString("mqtt_host", doc["mqtt"] | "");
                local_prefs.putInt("mqtt_port", doc["m_port"] | 1883);
                local_prefs.putString("mqtt_user", doc["m_user"] | "");
                local_prefs.putString("mqtt_pass", doc["m_pass"] | "");
                
                local_prefs.end();
                app_log("Configuration saved successfully. Rebooting device...");
                
                server.send(200, "text/plain", "OK");
                delay(1000);
                request_reboot();
                return;
            }
        }
        server.send(400, "text/plain", "Bad Request");
    });

    server.on("/api/calibrate_north", HTTP_POST, []() {
        if (!has_as5600) {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"AS5600 sensor not detected\"}");
            return;
        }
        uint16_t raw = read_as5600_raw_angle();
        if (raw == 0xFFFF) {
            server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to read from AS5600 sensor\"}");
            return;
        }
        int new_offset = (int)(raw * 360.0f / 4096.0f + 0.5f) % 360;

        wind_dir_offset = new_offset;
        prefs.begin("weather", false);
        prefs.putInt("w_dir_off", wind_dir_offset);
        prefs.end();

        app_log("[Wind] Automatic North calibration. Offset set to %d degrees.", wind_dir_offset);

        JsonDocument doc;
        doc["status"] = "success";
        doc["offset"] = wind_dir_offset;
        String res; serializeJson(doc, res);
        server.send(200, "application/json", res);
    });

    server.on("/api/calibrate_lux_unfiltered", HTTP_POST, []() {
        if (!has_bh1750) {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"BH1750 sensor not detected\"}");
            return;
        }
        Wire.requestFrom(0x23, 2);
        if (Wire.available() >= 2) {
            uint16_t raw = Wire.read();
            raw <<= 8;
            raw |= Wire.read();
            unfiltered_lux_ref = (float)raw / 1.2f;
            app_log("[Lux] Measured unfiltered reference: %s lx", String(unfiltered_lux_ref, 1).c_str());
            JsonDocument doc;
            doc["status"] = "success";
            doc["unfiltered_lux"] = unfiltered_lux_ref;
            String res; serializeJson(doc, res);
            server.send(200, "application/json", res);
        } else {
            server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to read from BH1750 sensor\"}");
        }
    });

    server.on("/api/calibrate_lux_filtered", HTTP_POST, []() {
        if (!has_bh1750) {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"BH1750 sensor not detected\"}");
            return;
        }
        if (unfiltered_lux_ref < 1.0f) {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Unfiltered reference reading is missing or too low\"}");
            return;
        }
        Wire.requestFrom(0x23, 2);
        if (Wire.available() >= 2) {
            uint16_t raw = Wire.read();
            raw <<= 8;
            raw |= Wire.read();
            float filtered_lux = (float)raw / 1.2f;
            float factor = filtered_lux / unfiltered_lux_ref;
            if (factor < 0.01f) factor = 0.01f;
            if (factor > 1.0f) factor = 1.0f;
            lux_cal_factor = factor;
            prefs.begin("weather", false);
            prefs.putFloat("lux_cal", lux_cal_factor);
            prefs.end();
            app_log("[Lux] Transmission calibration completed. Factor: %s (Transmission: %s%%)", 
                    String(lux_cal_factor, 4).c_str(), String(lux_cal_factor * 100.0f, 1).c_str());
            JsonDocument doc;
            doc["status"] = "success";
            doc["factor"] = lux_cal_factor;
            doc["transmission_pct"] = lux_cal_factor * 100.0f;
            String res; serializeJson(doc, res);
            server.send(200, "application/json", res);
        } else {
            server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to read from BH1750 sensor\"}");
        }
    });

    server.on("/api/reboot", HTTP_POST, []() {
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Rebooting...\"}");
        app_log("Reboot requested via Web UI. restarting...");
        delay(1000);
        request_reboot();
    });

    server.on("/api/crash", []() {
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Triggering crash exception...\"}");
        app_log("Crash requested via Web API. Dereferencing nullptr to force hardware exception...");
        delay(500);
        volatile int* ptr = nullptr;
        *ptr = 0xDEADBEEF; // Hardware StoreProhibited exception
    });

    server.on("/api/logs", HTTP_GET, []() {
        JsonDocument doc;
        for (int i = 0; i < log_buffer_count; i++) {
            int idx = (log_buffer_head - log_buffer_count + i + LOG_BUFFER_SIZE) % LOG_BUFFER_SIZE;
            doc.add(log_buffer[idx]);
        }
        String res; serializeJson(doc, res);
        server.send(200, "application/json", res);
    });

    server.on("/api/command", HTTP_POST, []() {
        if (server.hasArg("cmd")) {
            String cmd = server.arg("cmd");
            String response = execute_console_command(cmd);
            if (response != "" && response != "OK") {
                app_log("Resp: %s", response.c_str());
            }
            server.send(200, "text/plain", response);
        } else {
            server.send(400, "text/plain", "Missing cmd");
        }
    });

    server.on("/api/clear_counters", HTTP_POST, []() {
        total_bucket_tips = 0;
        last_processed_tips = 0;
        total_rain_mm = 0.0;
        memset(rain_history, 0, sizeof(rain_history));
        rolling_rain_hour = 0.0;
        rolling_rain_day = 0.0;
        
        prefs.begin("weather", false);
        prefs.putLong("tips", 0);
        prefs.end();
        
        app_log("Rain counters reset via WebUI.");
        server.send(200, "text/plain", "OK");
    });

    // OTA firmware upload
    server.on("/api/ota", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        delay(1000);
        request_reboot();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            app_log("OTA update started: %s", upload.filename.c_str());
            if (!Update.begin(ESP.getFreeSketchSpace())) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
                app_log("OTA update completed! Size: %u bytes", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });

    server.on("/api/factory_reset", HTTP_POST, []() {
        server.send(200, "text/plain", "OK");
        app_log("Factory reset initiated...");
        delay(500);

        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);

        prefs.begin("weather", false);
        prefs.clear();
        prefs.end();
        delay(500);
        ESP.restart();
    });

    server.begin();
    app_log("HTTP server started on port 80");
}

void handle_web_server() {
    server.handleClient();
}
