#!/usr/bin/env python3
"""Simple serial dashboard and CSV logger for SmartStreetLight.ino."""
import argparse
import csv
import datetime as dt
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("Missing dependency. Run: python -m pip install -r requirements.txt")
    raise SystemExit(1)

EXPECTED_HEADER = [
    "ms", "dark", "pir1", "pir2", "pir3",
    "count1", "count2", "count3", "dir",
    "pwm1", "pwm2", "pwm3",
    "voltage_V", "current_mA", "power_mW", "mode"
]

def choose_port():
    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found. Check the ESP32 USB cable/driver.")
        raise SystemExit(2)
    print("Available serial ports:")
    for i, p in enumerate(ports, 1):
        print(f"  {i}. {p.device} — {p.description}")
    while True:
        choice = input("Select port number: ").strip()
        try:
            return ports[int(choice) - 1].device
        except (ValueError, IndexError):
            print("Enter one of the listed numbers.")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", nargs="?", help="Serial port, e.g. COM5 or /dev/ttyUSB0")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--csv", default="streetlight_log.csv", help="Output CSV filename")
    args = parser.parse_args()
    port = args.port or choose_port()
    print(f"Opening {port} at {args.baud} baud. Press Ctrl+C to stop.")
    print(f"CSV log: {args.csv}")
    try:
        ser = serial.Serial(port, args.baud, timeout=2)
    except Exception as exc:
        print(f"Could not open serial port: {exc}")
        raise SystemExit(3)

    time.sleep(2)  # ESP32 may reset when serial opens
    log_file = open(args.csv, "a", newline="", encoding="utf-8")
    writer = csv.writer(log_file)
    if log_file.tell() == 0:
        writer.writerow(["logged_at"] + EXPECTED_HEADER)
        log_file.flush()

    try:
        while True:
            raw = ser.readline().decode("utf-8", errors="replace").strip()
            if not raw:
                continue
            if raw.startswith("ms,"):
                print("\nFirmware header detected.")
                continue
            parts = raw.split(",")
            if len(parts) != len(EXPECTED_HEADER):
                print(raw)
                continue
            row = dict(zip(EXPECTED_HEADER, parts))
            try:
                p1, p2, p3 = (int(row[k]) for k in ("pwm1", "pwm2", "pwm3"))
                counts = (int(row[k]) for k in ("count1", "count2", "count3"))
                c1, c2, c3 = counts
            except ValueError:
                print(raw)
                continue

            stamp = dt.datetime.now().isoformat(timespec="seconds")
            writer.writerow([stamp] + parts)
            log_file.flush()
            voltage = row["voltage_V"]
            current = row["current_mA"]
            power = row["power_mW"]
            print(
                f"\rDark={row['dark']} | activity=[{c1:2d}, {c2:2d}, {c3:2d}] "
                f"| direction={row['dir']:>6s} | PWM=[{p1:3d}%, {p2:3d}%, {p3:3d}%] "
                f"| V={voltage} V I={current} mA P={power} mW",
                end="", flush=True
            )
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        log_file.close()
        ser.close()
        print(f"Saved log to {args.csv}")

if __name__ == "__main__":
    main()
