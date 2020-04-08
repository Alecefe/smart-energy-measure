
var id_campos = ['ssid', 'password', 'meshID', 'mesh_password', 'max_layer', 'max_sta', 'port'];

function $(arg){
    try{return document.getElementById(String(arg));}
    catch(error){console.log(error);}
}

function reset_form(){
    for (i in id_campos){
         $(id_campos[i]).value = '';
    }
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

    // myObj = { "ssid":null, "password":null, "meshID":null, "mesh_password":null, "max_layer":null,
    // "max_sta":null, "port":null};

    req.onreadystatechange = function(){
        if(this.readyState==4 && this.status==200){
            console.log(this.responseText);
            if(this.responseText==='Error'){alert('Error almacenando la configuración')};
            if(this.responseText==='Success'){alert('Data almacenada satisfactoriamente')};
            
        }
    }

    for( i in myObj){
        myObj[i] = $(i).value;
    }

    myObj_string = JSON.stringify(myObj);

    req.open("POST", "/req_mesh", true);
    req.send(myObj_string);
}

function validar_form_mesh(){
    var valor_campos = {'ssid':null, 'password':null, 'meshID':null, 'mesh_password':null, 'max_layer':null, 'max_sta':null, 'port':null}, re;

    valor_campos.ssid = $('ssid').value;
    valor_campos.password = $('password').value;
    valor_campos.meshID = $('meshID').value;
    valor_campos.mesh_password = $('mesh_password').value;
    valor_campos.max_layer = $('max_layer').value;
    valor_campos.max_sta = $('max_sta').value;
    valor_campos.port = $('port').value;

    re = /^[0-9a-fA-Z]{1,2}([\.:-])(?:[0-9a-fA-Z]{1,2}\1){4}[0-9a-fA-Z]{1,2}$/

    for(i in valor_campos){
        if (valor_campos[i]===''){
             alert('Todos los campos son requeridos');
             return false;
        }
    }
    
    if(String(valor_campos.ssid).length >20){
        alert('El SSID debe contener menos de 20 caracteres');
        $('ssid').value = '';
        return false;

    }else if(String(valor_campos.password).length >20){
        alert('La contraseña debe contener menos de 20 caracteres');
        $('password').value = '';
        return false;

    }else if(!re.test(valor_campos.meshID)){
        alert('Formato de mesh ID incorrecto');
        $('meshID').value='';
        return false;
    }else if(re.test(valor_campos.meshID)){
        $('meshID').value = String(valor_campos.meshID).toLowerCase();

    }else if(String(valor_campos.mesh_password).length >15 || String(valor_campos.mesh_password).length < 6){
        alert('La contraseña de la mesh debe contener entre 6 y 15 caracteres');
        $('mesh_password').value='';
        return false;

    }else if(isNaN(parseInt(valor_campos.max_layer))){
        alert('Introduzca un Max. Layer válido');
        $('max_layer').value='';
        return false;
    }else if(parseInt(valor_campos.max_layer)>25 || parseInt(valor_campos.max_layer)<10){
        alert('El número de capas de la red debe estar entre 10 y 25')
        if (valor_campos.max_layer<10){$('max_layer').value='10';}
        if (valor_campos.max_layer>25){$('max_layer').value='25';}
        return false;
    }else if(isNaN(parseInt(valor_campos.max_sta))){
        alert('Introduzca un número de STA válido');
        $('max_sta').value='';$('max_sta').value='';
        return false;
    }else if(parseInt(valor_campos.max_sta)>9 || parseInt(valor_campos.max_sta)<1){
        alert('El número de STA a conectar debe estar entre 1 y 9')
        if(valor_campos.max_sta>9){$('max_sta').value='9';}
        if(valor_campos.max_sta<1){$('max_sta').value='1';}
        return false;
    }else if(isNaN(parseInt(valor_campos[port]))){
        alert('Introduzca un número de puerto válido');
        $('port').value='';
        return false;
    }else if(parseInt(valor_campos.port)>65536 || parseInt(valor_campos.port)<0){
        alert('Introduzca un puerto de conexión entre 0 y 65536')
        if (valor_campos.port > 65536){$('port').value='65536'}
        if (valor_campos.port < 0){$('port').value='0'}
        return false;
    }
    console.log(valor_campos);
    req_mesh();
}