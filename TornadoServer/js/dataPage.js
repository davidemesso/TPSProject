var storicTable   = null;
var realTimeTable = null;

var temperatureChart = null;
var pressureChart    = null;
var humidityChart    = null;
var ws = null;

// Callback to when the page is fully loaded, used to setup the whole page
$(document).ready(() => {
    initJQueryComponents();
    initTables();
    initCharts();
    initWS();
});

function initJQueryComponents()
{
    $('.datepicker').datepicker({
        dateFormat: 'yy-mm-dd'
    });

    $("#backButton").button({
        icons: {
            primary: "ui-icon-triangle-1-w"
        }
    });
}

function initTables()
{
    realTimeTable = new DataTable("realtimeDataTable", {
        responsive: true,
        "pageLength": 25,
        "dom": "tp"
    });

    storicTable = new DataTable('dataTable', {
        responsive: true,
        "pageLength": 25
    });
}

function initCharts()
{
    $("#tableContainer").hide();

    temperatureChart = new SensorGraph("temperatureChart");
    pressureChart    = new SensorGraph("pressureChart");
    humidityChart    = new SensorGraph("humidityChart");
}

function initWS()
{
    ws = new WSHandler("ws://localhost:8888/data/ws", wsOnMessage);
}

function wsOnMessage(evt) {
    let data = JSON.parse(evt.data);

    if(!Array.isArray(data))
    {   
        temperatureChart.plotData(data, data.temp);
        pressureChart.plotData(data, data.pres);
        humidityChart.plotData(data, data.hum);

        realTimeTable.addData(data);
    }
    else 
        showStoricDataField(data);
}

function showStoricDataField(data)
{
    $("#tableContainer").show();
    storicTable.clear();
    storicTable.addList(data)
}

function sendStoricDataRequest()
{
    let startTime = $("#startDatePicker").val();
    let endTime = $("#endDatePicker").val();

    let message = {
        type : "StoricDataRequest",
        payload: {
            startTime: startTime,
            endTime: endTime
        }
    }
    
    if(startTime && endTime)
        ws.send(JSON.stringify(message));
}

function closeStoricDataField()
{
    storicTable.clear();
    $("#tableContainer").hide();
}