import serial
import threading
import time
import sys
from datetime import datetime

SERIAL_PORT = '/dev/ttyAMA0'  
BAUD_RATE = 9600

# Global Threading Synchronizers
print_lock = threading.Lock()         
serial_lock = threading.Lock()        
auto_loop_active = threading.Event()  # Replaces START_AUTO_LOOP entirely

try:
    lora = serial.Serial(
        port=SERIAL_PORT,
        baudrate=BAUD_RATE,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=0.1
    )
except Exception as e:
    print(f"Error opening serial port: {e}")
    sys.exit()

def receive_thread():
    buffer = ""
    while True:
        try:
            # 1. Thread-safe read check
            with serial_lock:
                bytes_available = lora.in_waiting

            if bytes_available > 0:
                with serial_lock:
                    data = lora.read(bytes_available).decode('utf-8', errors='ignore')
                
                # DEBUG Output
                with print_lock:
                    sys.stdout.write('\r\x1b[K') 
                    sys.stdout.write(f"[RAW SERIAL DATA]: {repr(data)}\n")
                    sys.stdout.flush()

                buffer += data
                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    clean_line = line.strip()
                    if clean_line:
                        with print_lock:
                            sys.stdout.write('\r\x1b[K')  
                            print(f"[LoRa Response]: {clean_line}")
                            
                            if not auto_loop_active.is_set():
                                sys.stdout.write("Enter Setup AT Command (or type 'RUN IN LOOP'): ")
                            else:
                                sys.stdout.write("Enter Manual AT Command (Looping Active): ")
                            sys.stdout.flush()
        except Exception as e:
            with print_lock:
                print(f"\n[Receiver Error]: {e}")
        time.sleep(0.01)

def auto_sender_thread():
    while True:
        try:
            # 2. Safely check the event flag state
            if auto_loop_active.is_set():
                current_time = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                message_to_send = f"Time:{current_time}|Count:123456789"
                length = len(message_to_send)
                
                command = f"AT+SEND=2,{length},{message_to_send}\r\n"
                
                # 3. Thread-safe transmission block
                with serial_lock:
                    lora.write(command.encode('utf-8'))
                
                time.sleep(1)
            else:
                time.sleep(0.2)
        except Exception as e:
            time.sleep(1)

def main():
    print("==========================================================")
    print("--- LORA ACTIVATED SYSTEM READY   ---")
    print("==========================================================\n")

    recv_t = threading.Thread(target=receive_thread, daemon=True)
    recv_t.start()
    
    sender_t = threading.Thread(target=auto_sender_thread, daemon=True)
    sender_t.start()
    
    # PHASE 1: CONFIGURATION
    while True:
        user_input = input("Enter Setup AT Command (or type 'RUN IN LOOP'): ").strip()
        
        if len(user_input) > 0:
            if user_input.upper() == "RUN IN LOOP":
                break  
            
            command_to_execute = user_input + "\r\n"
            with serial_lock:
                lora.write(command_to_execute.encode('utf-8'))
            time.sleep(0.4)  
            print("")

    print("\n==================================================")
    print("automatic data started")
    print("==================================================\n")
    
    # 4. Safely activate background transmissions
    auto_loop_active.set()
    
    # PHASE 2: OPERATION
    while True:
        try:
            user_input = input("Enter Manual AT Command (Looping Active): ").strip()
            if len(user_input) > 0:
                command_to_execute = user_input + "\r\n"
                with print_lock:
                    sys.stdout.write(f"[Manual Command Sent]: {user_input}\n")
                with serial_lock:
                    lora.write(command_to_execute.encode('utf-8'))
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\nShutting down LoRa program...")
            lora.close()
            break

if __name__ == "__main__":
    main()