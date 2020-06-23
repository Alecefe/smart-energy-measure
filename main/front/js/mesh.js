function $(arg){
    try{return document.getElementById(String(arg));}
    catch(error){console.log(error);}
}

const formMesh = $('form-mesh');

eventListeners();

function eventListeners(){

    formMesh.addEventListener('submit', validarMesh);

}

function validarMesh(e){

    e.preventDefault();

    let meshValues = {meshid:null, meshpass:null, maxlayer:null, maxsta:null, port:null};

    re = /^[0-9a-fA-Z]{1,2}([\.:-])(?:[0-9a-fA-Z]{1,2}\1){4}[0-9a-fA-Z]{1,2}$/

    if($('mesh-id').value === ''){  
        alert('Introduce a valid mesh ID');
        return false;
    }else if(!re.test($('mesh-id').value)){
        alert('Formato de mesh ID incorrecto');
        $('mesh-id').value='';
        return false;
    }else{
        meshValues.meshid = $('mesh-id').value;
        meshValues.meshid = String(meshValues.meshid).toLowerCase();
        $('mesh-id').value = meshValues.meshid;
    }

    if($('mesh-password').value === ''){
        alert('Introduce a valid mesh password');
        return false;
    }else(meshValues.meshpass = $('mesh-password').value)

    if($('max-layer').value === ''){
        alert('Introduce a valid number of layers');
        return false;
    }else(meshValues.maxlayer = $('max-layer').value)

    if($('max-sta').value === ''){
        alert('Introduce a valid number of stations');
        return false;
    }else(meshValues.maxsta = $('max-sta').value)

    if($('port').value === ''){
        alert('Introduce a valid number of port');
        return false;
    }else(meshValues.port = $('port').value)

    send_mesh_form(meshValues);
}

function send_mesh_form(meshValues){
    
    var req = new XMLHttpRequest();

    if(this.readyState==4 && this.status==200){

        let respuesta = this.responseText;

        if(respuesta === 'safe'){
            alert('Succesfully safe the Mesh Data');
        }else{
            alert('There is a mistake in the form');
        }
    }

    myObj_string = JSON.stringify(meshValues);
    console.log(myObj_string);

    req.open("POST", "/mesh", true);
    req.send(myObj_string);

}

function cargarMesh(){
 
    req = new XMLHttpRequest();

    req.onreadystatechange = function(){

        if(this.readyState==4 && this.status==200){

            let respuesta = JSON.parse(this.responseText);
            
            $('mesh-id').value = respuesta.meshid;
            $('mesh-password').value = respuesta.meshpass;
            $('max-layer').value = respuesta.maxlayer;
            $('max-sta').value = respuesta.maxsta;
            $('port').value = respuesta.port;

        }

    }

    req.open("GET", "/mesh", true);
    req.send();

}

document.addEventListener('DOMContentLoaded',cargarMesh);
