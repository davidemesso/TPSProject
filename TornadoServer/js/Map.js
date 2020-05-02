class Map
{
    constructor() 
    {
        this.ostMap = {};

        this.ostMap.map = L.map('mapContainer');
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png')
            .addTo(this.ostMap.map);

        this.createMarker();
    }

    createMarker()
    {
        this.ostMap.marker = L
            .marker([0, 0])
            .addTo(this.ostMap.map);
    }

    markPosition(data, zoom)
    {
        let id = data.id;
        let lat = data.lat;
        let long = data.long;

        this.ostMap
            .map
            .setView([lat, long], zoom);
    
        this.ostMap
            .marker
            .setLatLng([lat, long])
            .bindPopup("ID: " + id)
            .openPopup();
    }
}