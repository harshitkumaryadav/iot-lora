import serial
import time
from datetime import datetime

# Initialize connection to RYLR998 on Pi 5
ser = serial.Serial('/dev/ttyAMA0', baudrate=115200, timeout=1.5)

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
    
    # 2. Listen for the instant return echo from the Arduino
    start_wait = time.time()
    packet_received = False
    
    while (time.time() - start_wait) < 1.5:  # 1.5 second window to catch echo
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
        print(f"Node {target_node} failed to echo back in time (Packet Loss).")

try:
    print(f"Starting continuous loop testing... Writing directly to {FILE_NAME}")
    test_data = "123456789"
    
    while True:
        # Step A: Ping Arduino 1 (Address 1)
        run_latency_loop(1, test_data)
        time.sleep(1)  # 1-second pause to let the airwaves clear
        
        # Step B: Ping Arduino 2 (Address 2)
        run_latency_loop(2, test_data)
        time.sleep(1)  # 1-second pause before starting the loop over again

except KeyboardInterrupt:
    ser.close()
    print("\nLoop stopped by user.")