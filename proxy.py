from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib.request

ESP32_IP = "10.89.61.198"  # ← change this to what your device shows on screen

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        target = f"http://{ESP32_IP}{self.path}"
        print(f"Forwarding to: {target}")
        try:
            resp = urllib.request.urlopen(target)
            body = resp.read()
            self.send_response(200)
            self.end_headers()
            self.wfile.write(body)
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(str(e).encode())
    def log_message(self, format, *args):
        pass  # suppress noise

print("Proxy running on http://127.0.0.1:8888")
HTTPServer(('127.0.0.1', 8888), Handler).serve_forever()