var sensorTable = null;
var map = null;
var ws = null;

// Callback to when the page is fully loaded, used to setup the whole page
$(document).ready(() => {
    initJQueryComponents();
    initTable();
    initMap();
    initWS();
    sendSensorsDataRequest();
});

function initJQueryComponents()
{
    $("#backButton").button({
        icons: {
            primary: "ui-icon-triangle-1-w"
        }
    });
}

function initTable()
{
    sensorTable = new DataTable("sensorsDataTable", {
        responsive: true,
        "pageLength": 25,
        "dom": "tp"
    });
}

function initMap()
{
    map = new Map();
}

function initWS()
{
    ws = new WSHandler("ws://localhost:8888/data/ws", wsOnMessage);
}

function wsOnMessage(evt) {
    let data = JSON.parse(evt.data);
    showSensorsDataField(data);

    map.markPositions(data, 12);
}

function showSensorsDataField(data)
{
    if(Array.isArray(data))
    {
        $("#tableContainer").show();
        sensorTable.clear();
        sensorTable.addList(data)
    }
}

function sendSensorsDataRequest()
{   
    packet = {
        type : "SensorsDataRequest",
        payload : {} 
    };

    setTimeout(() => {
        ws.send(JSON.stringify(packet));
    }, 500);
}