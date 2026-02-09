#!/usr/bin/env python3
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import base64

class RedfishHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # 檢查基本認證
        auth = self.headers.get('Authorization')
        if auth:
            auth_type, credentials = auth.split(' ', 1)
            if auth_type == 'Basic':
                decoded = base64.b64decode(credentials).decode('utf-8')
                username, password = decoded.split(':', 1)
                if username != 'admin' or password != 'password':
                    self.send_error(401, 'Unauthorized')
                    return
        
        # 處理不同路徑
        if self.path == '/redfish/v1/Systems/1':
            response = {
                "Id": "1",
                "Name": "System",
                "Manufacturer": "Advantech",
                "Model": "BMC-Test-System",
                "SerialNumber": "12345678",
                "PowerState": "On",
                "BiosVersion": "1.0.0"
            }
            self.send_json_response(response)
            
        elif self.path.startswith('/redfish/v1/Chassis/') and self.path.endswith('/Thermal'):
            response = {
                "Id": "Thermal",
                "Name": "Thermal",
                "Temperatures": [
                    {
                        "Name": "CPU Temperature",
                        "ReadingCelsius": 45.0,
                        "Status": {"State": "Enabled", "Health": "OK"}
                    },
                    {
                        "Name": "System Temperature",
                        "ReadingCelsius": 35.0,
                        "Status": {"State": "Enabled", "Health": "OK"}
                    }
                ],
                "Fans": [
                    {
                        "Name": "System Fan 1",
                        "Reading": 3000,
                        "ReadingUnits": "RPM"
                    }
                ]
            }
            self.send_json_response(response)
            
        else:
            self.send_error(404, 'Not Found')
    
    def send_json_response(self, data):
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(data, indent=2).encode('utf-8'))
    
    def log_message(self, format, *args):
        print(f"[Redfish] {self.address_string()} - {format % args}")

def run_server(port=8000):
    server = HTTPServer(('127.0.0.1', port), RedfishHandler)
    print("=" * 50)
    print(f"Mock Redfish Server running on http://127.0.0.1:{port}")
    print("Username: admin")
    print("Password: password")
    print("Press Ctrl+C to stop")
    print("=" * 50)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n\nStopping server...")
        server.shutdown()

if __name__ == '__main__':
    run_server()
