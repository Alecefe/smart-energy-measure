function $(arg){
    try{return document.getElementById(String(arg));}
    catch(error){console.log(error);}
}

const formLocWifi = $('form-loc-wifi');

eventListeners();

function eventListeners(){
    formLocWifi.addEventListener('submit', validarLocWifi);

}

function validarLocWifi(e){
    
    e.preventDefault();

    let locWifiValues = {ssid:null, pass:null};

    if($('net-wifi-ssid').value === ''){
        alert('Introduce your SSID');
        return false;
    }else{locWifiValues.ssid = $('net-wifi-ssid').value;}

    if($('net-wifi-pass').value === ''){
        alert('Introduce your password');
        return false;
    }else{locWifiValues.pass = $('net-wifi-pass').value;}

    send_locwifi_form(locWifiValues);
}

function send_locwifi_form(locWifi_Values){

    var req = new XMLHttpRequest();

    if(this.readyState==4 && this.status==200){

        let respuesta = this.responseText;
         
        if(respuesta === 'safe'){
            alert('Succesfully safe the local Wifi Data');
        }else{
            alert('There is a mistake in the form');
        }
    }

    myObj_string = JSON.stringify(locWifi_Values);
    console.log(myObj_string);
 
    req.open("POST", "/locwifi", true);
    req.send(myObj_string);
}

function cargarLocWifi(){
 
    req = new XMLHttpRequest();

    req.onreadystatechange = function(){

        if(this.readyState==4 && this.status==200){

            let respuesta = JSON.parse(this.responseText);

            $('net-wifi-ssid').value = respuesta.ssid;
            $('net-wifi-pass').value = respuesta.pass;

        }

    }

    req.open("GET", "/locwifi", true);
    req.send();

}

document.addEventListener('DOMContentLoaded',cargarLocWifi);
