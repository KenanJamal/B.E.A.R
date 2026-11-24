import sys
import time
import json
import serial

# usage: python send_json.py COM3 output/english.json
port = sys.argv[1] if len(sys.argv) > 1 else "COM3"
json_path = sys.argv[2] if len(sys.argv) > 2 else "output/english.json"

with open(json_path, "r", encoding="utf-8") as f:
    ##remove the spaces and new lines so it sends ~3x faster
    data = json.dumps(json.load(f), separators=(",", ":"))

with serial.Serial(port, 9600, timeout=1) as ser:
    ##opening the port resets the arduino, wait until it says it's ready
    print(f"waiting for arduino on {port}...")
    start = time.time()
    while True:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if line:
            print("arduino:", line)
        if line.startswith("Ready"):
            break
        if time.time() - start > 10:
            print("arduino never said Ready - check the port and that the sketch is uploaded")
            sys.exit(1)

    print(f"sending {json_path} ({len(data)} characters)...")
    ser.write(data.encode("utf-8"))
    ser.flush()

    ##print what the arduino says while it shows every letter,
    ##stop when it has been quiet for 10 seconds (each letter takes 3.5s)
    last_line = time.time()
    while time.time() - last_line < 10:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if line:
            print("arduino:", line)
            last_line = time.time()
    print("done")
