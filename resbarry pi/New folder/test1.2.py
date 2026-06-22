import serial
import time
import csv
from datetime import datetime
import os

CSV_FILE_NAME = "lora_data_log.csv"

# रास्पबेरी पाई UART सेटअप
ser = serial.Serial('/dev/ttyS0', 115200, timeout=1)

nodes_tracker = {
    1: {"expected_seq": None, "total_lost": 0},
    2: {"expected_seq": None, "total_lost": 0}
}

def initialize_csv():
    if not os.path.exists(CSV_FILE_NAME):
        with open(CSV_FILE_NAME, mode='w', newline='', encoding='utf-8') as file:
            writer = csv.writer(file)
            writer.writerow(["Timestamp", "Node_ID", "Sequence_No", "Data", "Status"])
        print(f"📁 नई लॉग फाइल '{CSV_FILE_NAME}' बना दी गई है।")

def log_to_csv(node_id, seq_no, data, status):
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(CSV_FILE_NAME, mode='a', newline='', encoding='utf-8') as file:
        writer = csv.writer(file)
        writer.writerow([current_time, node_id, seq_no, data, status])

def process_rylr_packet(raw_line):
    # उदाहरण रॉ डेटा: +RCV=1,23,1,0,1,2,3,4,5,6,7,8,9,215,-45,10
    
    if not raw_line.startswith("+RCV="):
        return 

    try:
        # क्लीन करके कोमा से स्प्लिट करें
        parts = raw_line.replace("+RCV=", "").strip().split(',')
        
        # डायनेमिक इंडेक्सिंग ताकि डेटा के बीच के कोमा गड़बड़ न करें
        node_id = int(parts[2])
        seq_no = int(parts[3])
        
        # RSSI और SNR हमेशा आखिरी दो एलिमेंट्स होते हैं (parts[-2] और parts[-1])
        # चेकसम हमेशा RSSI से ठीक पहले होता है
        incoming_checksum = int(parts[-3])
        
        # बीच का पूरा हिस्सा हमारा फिक्स डेटा है (parts[4] से लेकर parts[-3] के ठीक पहले तक)
        sensor_data_list = parts[4:-3]
        sensor_data = ",".join(sensor_data_list) # वापस "1,2,3,4,5,6,7,8,9" स्ट्रिंग बन जाएगी
        
        # चेकसम वेरिफिकेशन के लिए ओरिजिनल स्ट्रिंग दोबारा बनाना (जैसा Arduino ने बनाया था)
        original_string = f"{node_id},{seq_no},{sensor_data}"
        
        total_sum = 0
        for char in original_string:
            total_sum += ord(char)
        calculated_checksum = total_sum % 256

        # 1. चेकसम चेक
        if calculated_checksum != incoming_checksum:
            print(f"❌ [CHECKSUM ERROR] Node {node_id} | पैकेट #{seq_no} का चेकसम फेल!")
            print(f"कैलकुलेटेड: {calculated_checksum} | आया: {incoming_checksum}")
            log_to_csv(node_id, seq_no, "Corrupted Data", "CHECKSUM_FAIL")
            return

        if node_id not in nodes_tracker:
            print(f"⚠️ अनजान Node ID: {node_id} से डेटा मिला।")
            log_to_csv(node_id, seq_no, sensor_data, "UNKNOWN_NODE")
            return

        node_status = nodes_tracker[node_id]
        packet_status = "OK"

        # 2. सीक्वेंस नंबर (पैकेट लॉस) चेक
        if node_status["expected_seq"] is None:
            print(f"📡 Node {node_id} कनेक्टेड। पहला पैकेट: #{seq_no}")
            packet_status = "FIRST_PACKET"
        else:
            if seq_no != node_status["expected_seq"]:
                if seq_no > node_status["expected_seq"]:
                    missing_count = seq_no - node_status["expected_seq"]
                else:
                    missing_count = (256 - node_status["expected_seq"]) + seq_no
                
                node_status["total_lost"] += missing_count
                print(f"⚠️ [PACKET LOSS] Node {node_id} के बीच में {missing_count} पैकेट मिस हुए!")
                
                # मिस हुए पैकेट्स को CSV में दर्ज करें
                for m in range(missing_count):
                    missed_num = (node_status["expected_seq"] + m) % 256
                    log_to_csv(node_id, missed_num, "No Data", "MISSED")
                
                packet_status = "OK_AFTER_LOSS"

        # 3. सही डेटा को CSV में लिखें
        log_to_csv(node_id, seq_no, sensor_data, packet_status)
        print(f"📊 डेटा सेव हुआ -> Node: {node_id} | पैकेट: #{seq_no} | डेटा: [{sensor_data}]")
        node_status["expected_seq"] = (seq_no + 1) % 256
        print("-" * 60)

    except Exception as e:
        print(f"❌ पैकेट पार्सिंग एरर: {e} | रॉ डेटा था: {raw_line}")

if __name__ == "__main__":
    initialize_csv()
    
    # पाई के मॉड्यूल को एड्रेस 0 (Gateway/Receiver) पर सेट करें
    ser.write(b"AT+ADDRESS=0\r\n")
    time.sleep(0.5)
    
    print("🚀 RYLR998 रास्पबेरी पाई रिसीवर एक्टिव है...")
    
    while True:
        if ser.in_waiting > 0:
            try:
                raw_data = ser.readline().decode('utf-8', errors='ignore').strip()
                if raw_data:
                    process_rylr_packet(raw_data)
            except Exception as e:
                print(f"सीरियल रीड एरर: {e}")
        time.sleep(0.01)