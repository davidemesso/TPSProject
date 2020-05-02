class WSHandler{
    constructor(url, callback) 
    {
        this.ws = new WebSocket(url);
        this.ws.onmessage = callback;
    }

    send(data)
    {
        this.ws.send(data);
    }
}