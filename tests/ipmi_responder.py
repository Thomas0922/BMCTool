#!/usr/bin/env python3
import socket, struct

def calc_checksum(data):
    return (0x100 - (sum(data) & 0xFF)) & 0xFF

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('127.0.0.1', 9623))

print("=" * 50)
print("IPMI Responder on 127.0.0.1:9623")
print("Press Ctrl+C to stop")
print("=" * 50)

try:
    while True:
        data, addr = sock.recvfrom(1024)
        if len(data) >= 20 and data[19] == 0x01:  # Get Device ID
            seq = (data[18] >> 2) & 0x3F
            
            # RMCP Header
            rmcp = struct.pack('BBBB', 0x06, 0x00, 0xFF, 0x07)
            
            # Session Header
            session = struct.pack('B', 0x00)  # Auth type
            session += struct.pack('I', 0x00)  # Session sequence
            session += struct.pack('I', 0x00)  # Session ID
            
            # IPMI Message Header
            target_addr, netfn_lun = 0x20, 0x1C
            hdr_cs = calc_checksum([target_addr, netfn_lun])
            source_addr, seq_lun, cmd = 0x81, (seq << 2), 0x01
            
            # Data: Get Device ID Response (完整版)
            data_bytes = bytes([
                0x00,        # Completion code (success)
                0x20,        # Device ID
                0x00,        # Device Revision (bits 3:0)
                0x01,        # Firmware Rev 1
                0x00,        # Firmware Rev 2
                0x02,        # IPMI Version (2.0)
                0x07,        # Additional Device Support (修正：加上這個!)
                0xB4, 0x00, 0x00,  # Manufacturer ID (Advantech = 0x0000B4)
                0x00, 0x00   # Product ID
            ])
            
            # Data checksum
            data_cs = calc_checksum([source_addr, seq_lun, cmd] + list(data_bytes))
            
            # Message header
            msg_hdr = struct.pack('BBBBBB', 
                target_addr, netfn_lun, hdr_cs, 
                source_addr, seq_lun, cmd)
            
            msg_len = len(msg_hdr) + len(data_bytes) + 1
            
            # Build response
            resp = rmcp + session + bytes([msg_len]) + msg_hdr + data_bytes + bytes([data_cs])
            
            # Send
            sock.sendto(resp, addr)
            print(f"[{addr[0]}:{addr[1]}] Get Device ID (seq={seq}) -> Response sent ({len(resp)} bytes)")
            
except KeyboardInterrupt:
    print("\n\nStopped")
finally:
    sock.close()
    print("Socket closed")
