from flask import Flask

count = 0
connected_devices = {}
app = Flask(__name__)

@app.route('/')
def index():
    return "Good"

@app.route('/heartbeat')
def heartbeat():
    print "heartbeat Request Received"
    return "Good"

@app.route('/display')
def display():
    global connected_devices
    for item in connected_devices:
        print item, connected_devices[item]
    return "OK"

@app.route('/getChild')
def getChild():
    print "heartbeat Request Received"
    global count
    count = count + 1
    return str(count)

@app.route('/sendToRoot/<int:dev_id>/<int:status>', methods=['GET'])
def sendToRoot(dev_id, status):
    global connected_devices
    print dev_id, status
    connected_devices[dev_id] = status

    print "sendToRoot Request Received"
    #Update Webpage
    return "Good"

if __name__ == '__main__':
    app.run(host= '172.24.24.144', debug=True)