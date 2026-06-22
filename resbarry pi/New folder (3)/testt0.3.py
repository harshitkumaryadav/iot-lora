import serial  # Fixed: genuinely lowercase 'import' now!
import time
import csv
from datetime import datetime
import os

CSV_FILE_NAME = "lora_data_log.csv"

# Raspberry Pi UART Setup
ser = serial.Serial('/dev/ttyS0', 115200, timeout=1)

nodes_tracker = {
    1: {"expected_seq": None, "total_lost": 0},
    2: {"expected_seq": None, "total_lost": 0}
}

# Maximum packet loss threshold before assuming a node went out of range
MAX_ALLOWED_MISSING = 10 

def initialize_csv():
    if not os.path.exists(CSV_FILE_NAME):
        with open(CSV_FILE_NAME, mode='w', newline='', encoding='utf-8') as file:
            writer = csv.writer(file)
            writer.writerow(["Timestamp", "Node_ID", "Sequence_No", "Data", "Status"])
        print(f"📁 New log file '{CSV_FILE_NAME}' created successfully.")

def log_to_csv(node_id, seq_no, data, status):
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(CSV_FILE_NAME, mode='a', newline='', encoding='utf-8') as file:
        writer = csv.writer(file)
        writer.writerow([current_time, node_id, seq_no, data, status])

def process_rylr_packet(raw_line):
    if not raw_line.startswith("+RCV="):
        return 

    try:
        parts = raw_line.replace("+RCV=", "").strip().split(',')
        
        # Safety Check: Ensure essential structural elements exist
        if len(parts) < 6:
            return

        node_id = int(parts[2])
        seq_no = int(parts[3])
        
        incoming_checksum = int(parts[-3])
        sensor_data_list = parts[4:-3]
        sensor_data = ",".join(sensor_data_list) 
        
        # Checksum Verification
        original_string = f"{node_id},{seq_no},{sensor_data}"
        total_sum = 0
        for char in original_string:
            total_sum += ord(char)
        calculated_checksum = total_sum % 256

        if calculated_checksum != incoming_checksum:
            print(f"❌ [CHECKSUM ERROR] Node {node_id} | Packet #{seq_no} failed verification!")
            log_to_csv(node_id, seq_no, "Corrupted Data", "CHECKSUM_FAIL")
            return

        if node_id not in nodes_tracker:
            print(f"⚠️ [UNKNOWN NODE] Data received from unregistered Node ID: {node_id}")
            log_to_csv(node_id, seq_no, sensor_data, "UNKNOWN_NODE")
            return

        node_status = nodes_tracker[node_id]
        packet_status = "OK"

        # --- Packet Loss & Auto-Reconnection Logic ---
        if node_status["expected_seq"] is None:
            print(f"📡 Node {node_id} connected. Initial Packet Received: #{seq_no}")
            packet_status = "FIRST_PACKET"
        else:
            if seq_no != node_status["expected_seq"]:
                # Calculate the exact number of missing packets
                if seq_no > node_status["expected_seq"]:
                    missing_count = seq_no - node_status["expected_seq"]
                else:
                    missing_count = (256 - node_status["expected_seq"]) + seq_no
                
                # If packet loss exceeds threshold, the node was out of range
                if missing_count > MAX_ALLOWED_MISSING:
                    print(f"🔄 [RE-CONNECTED] Node {node_id} back in range! Sequence state synced safely.")
                    packet_status = "RECONNECTED"
                else:
                    # Minor packet loss due to momentary obstacles or distance dips
                    node_status["total_lost"] += missing_count
                    print(f"⚠️ [PACKET LOSS] Node {node_id} dropped {missing_count} packet(s)!")
                    
                    for m in range(missing_count):
                        missed_num = (node_status["expected_seq"] + m) % 256
                        log_to_csv(node_id, missed_num, "No Data", "MISSED")
                    
                    packet_status = "OK_AFTER_LOSS"

        # Commit current clean packet to CSV file
        log_to_csv(node_id, seq_no, sensor_data, packet_status)
        print(f"📊 Data Logged -> Node: {node_id} | Packet: #{seq_no} | Status: {packet_status}")
        
        # Track next expected sequence dynamically
        node_status["expected_seq"] = (seq_no + 1) % 256
        print("-" * 60)

    except Exception as e:
        print(f"❌ Packet Parsing Error: {e} | Raw string was: {raw_line}")

if __name__ == "__main__":
    initialize_csv()
    
    # Flush existing serial buffer data completely to start clean
    ser.reset_input_buffer()
    
    # Configure Gateway Module Address
    ser.write(b"AT+ADDRESS=0\r\n")
    time.sleep(0.5)
    
    print("🚀 RYLR998 Raspberry Pi Gateway/Receiver Active...")
    
    while True:
        if ser.in_waiting > 0:
            try:
                raw_data = ser.readline().decode('utf-8', errors='ignore').strip()
                if raw_data:
                    process_rylr_packet(raw_data)
            except Exception as e:
                print(f"Serial Port Read Error: {e}")
        time.sleep(0.01)