class DataTable
{
    constructor(id, options) 
    {    
        this.table = $('#'+id).DataTable(options);
    }

    addData(data)
    {
        this.table
            .row
            .add(Object.values(data))
            .draw(false);
    }

    addList(list)
    {
        for(let el of list)
            this.addData(el);
    }

    clear()
    {
        this.table
            .clear()
            .draw(false);
    }
}