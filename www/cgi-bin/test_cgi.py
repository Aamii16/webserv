#!/usr/bin/env python3
import os
import sys
import urllib.parse

# 1. Print valid HTTP headers required by the server/browser
print("Content-Type: text/html\r\n\r\n")

# 2. Gather environment variables sent by your webserver
request_method = os.environ.get("REQUEST_METHOD", "GET")
query_string = os.environ.get("QUERY_STRING", "")

# 3. Read Body if method is POST
body_data = ""
if request_method == "POST":
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        if content_length > 0:
            body_data = sys.stdin.read(content_length)
    except Exception as e:
        body_data = f"Error reading stdin: {str(e)}"

# 4. Generate HTML Response
print(f"""<!DOCTYPE html>
<html>
<head><title>CGI Execution Result</title></head>
<body style="font-family: monospace; padding: 20px; background: #222; color: #fff;">
    <h1 style="color: #00ff00;">✔ CGI Script Executed Successfully</h1>
    <hr>
    <h3>Server Environment Variables Received:</h3>
    <ul>
        <li><b>REQUEST_METHOD:</b> {request_method}</li>
        <li><b>QUERY_STRING:</b> {query_string}</li>
        <li><b>CONTENT_LENGTH:</b> {os.environ.get("CONTENT_LENGTH", "Not Set")}</li>
    </ul>
""")

# Parse parameters for display
if request_method == "GET" and query_string:
    parsed = urllib.parse.parse_qs(query_string)
    print(f"<h3>Parsed GET Data:</h3><p>{parsed}</p>")
elif request_method == "POST" and body_data:
    parsed = urllib.parse.parse_qs(body_data)
    print(f"<h3>Raw POST Body:</h3><p>{body_data}</p>")
    print(f"<h3>Parsed POST Data:</h3><p>{parsed}</p>")

print("</body></html>")
