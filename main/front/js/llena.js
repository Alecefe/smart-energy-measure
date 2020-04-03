
function $(arg){
    return document.getElementById(String(arg));
}

function solicitudMesh(){ 
    //Solicita y sube los datos actuales en la memoria del servidor y los postea en la página

    var req = new XMLHttpRequest();

    myObj = { "ssid":null, "password":null, "meshID":null, "mesh_password":null, "max_layer":null,
    "max_sta":null, "port":null};

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
    //Envía los datos actuales en el formulario al servidor

    var req = new XMLHttpRequest();

    myObj = { "ssid":null, "password":null, "meshID":null, "mesh_password":null, "max_layer":null,
    "max_sta":null, "port":null};

    req.onreadystatechange = function(){
        if(this.readyState==4 && this.status==200){

        }
    }

    for( i in myObj){
        myObj[i] = $(i).value;
    }

    myObj_string = JSON.stringify(myObj);

    req.open("POST", "/req_mesh", true);
    req.send(myObj_string);
}