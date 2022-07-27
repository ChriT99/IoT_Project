var values_x = [];
var coluomns_colors = []; 
var colors = ["black", "black", "black","black", "black","black", "black", "black","black", "black"]
var current_color = 0;
var values_y = [0, 0, 0, 0, 0, 0, 0];
var ch1;
    

function publish_topic_to_aws(msg){
    var myHeaders = new Headers();
    var raw = JSON.stringify({"message":msg});

    var requestOptions = {
        method: 'POST',
        headers: myHeaders,
        body: raw
    };

    fetch("https://mh13imsa5c.execute-api.eu-west-1.amazonaws.com/dev", requestOptions)
    .then(response => response.text())
    .then()
}


function displayDataOnChart(input){

    for(let i=0;i < input.length; i++){
        current_date=new Date();
        hour = current_date.getHours();  
        minute = current_date.getMinutes(); 
        //label = "Time: " + hour.toString()+":" + minute.toString(); 
        label = input[i][0];
        coluomns_colors.push(colors[current_color%3]); 
        current_color++;
        ch1.data.labels.push(label);  
        ch1.data.datasets[0].data[i] = input[i][1]; 
        ch1.update();
    }

}

function callAPI(){
    var myHeaders = new Headers();
    var requestOptions = {
        method: 'GET',
        headers: myHeaders,
    };
    
    fetch("https://0lwyeay63k.execute-api.eu-west-1.amazonaws.com/dev", requestOptions)
    .then(response => response.text())
    .then(result => displayDataOnChart(JSON.parse(result).body))
    
}

function createPlot(name, val_y){

    ch = new Chart(name, {
        type: "bar",
        data: {
            labels: values_x,
            datasets: [{
            backgroundColor: coluomns_colors,
            data: val_y
            }]
        },
        options: {
            legend: {display: false},
            title: {
            display: true,
            },
            scales: {
                yAxes: [{
                  ticks: {
                    beginAtZero: true
                  }
                }],
              }

        }
        });
    return ch;


}



function init(){   
    
    ch1=createPlot("Chart1", values_y);

    callAPI();

}
 