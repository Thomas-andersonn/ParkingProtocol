from flask import Flask
from flask import request

count = 0
connected_devices = {}
app = Flask(__name__)

@app.route('/')
def index():
    return "Good"

@app.route('/heartbeat')
def heartbeat():
    print ("heartbeat Request Received")
    return "Good"

@app.route('/display')
def display():
    global connected_devices
    for item in connected_devices:
        print (item, connected_devices[item])
    return "OK"

@app.route('/getChild')
def getChild():
    print ("heartbeat Request Received")
    global count
    count = count + 1
    return str(count)

@app.route('/sendToRoot', methods=['GET'])
def sendToRoot():
    global connected_devices
    dev_id = request.args.get('dev_id')
    status = request.args.get('status')
    print (dev_id, status)
    connected_devices[dev_id] = status

    print ("sendToRoot Request Received")
    #Update Webpage
    return "Good"

if __name__ == '__main__':
    app.run(host= '192.168.137.1',port = 80, debug=True)