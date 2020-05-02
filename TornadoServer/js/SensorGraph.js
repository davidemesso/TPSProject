class SensorGraph
{
    constructor(id)
    {
        this.chart = null;
        this.timeList = [];
        this.dataList = [];
        this.sensorList = [];
        this.colorForSensor = {};
        this.datasets = {};

        this.initChart(id);
    }

    initChart(id)
    {
        let chartContext = $("#"+id).get(0).getContext("2d");
        this.chart = new Chart(chartContext, {
            type: 'line'
        });
    }

    plotData(data, value)
    {
        let timestamp = this.getTimestamp(data);
        let color = this.getRandomColorString();

        if(!this.sensorList.includes(data.id))
        {
            this.sensorList.push(data.id);
            this.colorForSensor[data.id] = color;
            this.dataList[data.id] = [];
        }

        this.timeList.push(timestamp);
        this.dataList[data.id].push({x: timestamp, y: value});
        
        this.datasets[data.id] = {
            label: data.id,
            data: this.dataList[data.id],
            backgroundColor: [
                this.colorForSensor[data.id]+' 0.2)'
            ],
            borderColor: [
                this.colorForSensor[data.id]+' 1)'
            ],
            borderWidth: 1
        }					
        
        this.chart.data.labels = this.timeList;
        this.chart.data.datasets = $.map(this.datasets, function(value, i) {
            return [value];
        });
        this.chart.update();
    }

    getTimestamp(data)
    {
        let d = new Date(0);
        d.setUTCSeconds(data.time);
        return d.getFullYear()+"-"+(d.getMonth()+1)+"-"+d.getDate()+" "+
                d.getHours()+":"+d.getMinutes()+":"+d.getSeconds()
    }

    getRandomColorString()
    {
        return "rgba("+Math.floor(Math.random() * (255))+" ,"+
                    Math.floor(Math.random() * (255))+", "+
                    Math.floor(Math.random() * (255))+",";
    }
}