
function $(arg){
    return document.getElementById(String(arg));
}

function solicitudMesh(){

    var req = new XMLHttpRequest();

    req.onreadystatechange = function(){
        if(this.readyState==4 && this.status==200){

            entrada = JSON.parse(this.responseText);
            for( i in myObj){
                $(i).value = entrada[i];
            }
        }
    }
    req.open("GET", "/formMesh", true);
    req.send();
}

function req_mesh(){

    var req = new XMLHttpRequest();

    myObj = { "ssid":null, "password":null, "meshID":null, "mesh_password":null, "max_layer":null,
    "max_sta":null, "port":null};

    req.onreadystatechange = function(){
        if(this.readyState==4 && this.status==200){
            console.log(this.responseText);
        }
    }

    for( i in myObj){
        myObj[i] = $(i).value;
    }

    myObj_string = JSON.stringify(myObj);

    req.open("POST", "req_mesh", true);
    req.send(myObj_string);
}