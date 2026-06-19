import serial
import time
from datetime import datetime

# Initialize connection to RYLR998 on Pi 5
ser = serial.Serial('/dev/ttyAMA0', baudrate=115200, timeout=0.5) # Reduced timeout to 0.5s for faster cycling

# Name of your text file output
FILE_NAME = "data.txt"

def run_latency_loop(target_node, raw_data):
    # 1. Get current time in milliseconds
    t1 = int(time.time() * 1000)
    
    # Construct payload: "123456789,timestamp"
    message = f"{raw_data},{t1}"
    length = len(message)
    
    # Send AT command to LoRa Module
    command = f"AT+SEND={target_node},{length},{message}\r\n"
    ser.write(command.encode('utf-8'))
    
    # Clear the local "+OK" response from the Pi's own module instantly
    try:
        ser.readline()
    except Exception:
        pass

    # 2. Listen for the return echo from the Arduino
    start_wait = time.time()
    packet_received = False
    
    # We only wait a maximum of 0.4 seconds so we don't break our 1-second loop budget
    while (time.time() - start_wait) < 0.4:  
        if ser.in_waiting:
            response = ser.readline().decode('utf-8', errors='ignore')
            
            if "+RCV" in response:
                # 3. Calculate latency immediately upon reception
                t2 = int(time.time() * 1000)
                rtt = t2 - t1
                latency = rtt / 2
                packet_received = True
                
                # Format log entry
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                log_entry = f"[{timestamp}] Target: Node {target_node} | Payload: {raw_data} | RTT: {rtt}ms | Latency: {latency}ms\n"
                
                # Append to the text file
                with open(FILE_NAME, "a") as file:
                    file.write(log_entry)
                
                print(f"Logged to file -> {log_entry.strip()}")
                break
                
    if not packet_received:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Node {target_node} timed out (Packet Loss).")

try:
    print(f"Starting rapid 1-second loop testing... Writing to {FILE_NAME}")
    test_data = "123456789"
    
    while True:
        # Ping Node 1
        run_latency_loop(1, test_data)
        time.sleep(0.5)  # half-second delay
        
        # Ping Node 2
        run_latency_loop(2, test_data)
        time.sleep(0.5)  # half-second delay -> Total cycle time for both nodes = 1 second total!

except KeyboardInterrupt:
    ser.close()
    print("\nLoop stopped by user.")