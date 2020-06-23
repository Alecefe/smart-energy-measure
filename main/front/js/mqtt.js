
let mqtt_formulario = document.getElementById('form-mqtt');
mqtt_formulario.addEventListener('submit',mqtt_validar);

let mqtt_check_public = document.querySelector('.mqtt_broker_type>.custom-control>#customRadio1');
mqtt_check_public.addEventListener('input',mqtt_type_checked);

let mqtt_check_private = document.querySelector('.mqtt_broker_type>.custom-control>#customRadio2').addEventListener('input',mqtt_type_checked);

let mqtt_configuration = document.querySelector('.mqtt-uri-configuration>.custom-control>#customSwitch1');
mqtt_configuration.addEventListener('input',verifica_config);
mqtt_configuration.checked = false;

function mqtt_type_checked(){
    let mqtt_private = document.querySelector('.private_mqtt');

    if(mqtt_check_public.checked===true){

        mqtt_private.style.display = 'none';

    }else{

        mqtt_private.style.display = 'block';

    }
}

function verifica_config(){

    let mqtt_uri = document.querySelector('.mqtt-uri-watch');
    let mqtt_no_advance = document.querySelectorAll('.no_advance');

    if(this.checked === true){
        mqtt_uri.style.display = 'none';
        mqtt_no_advance.forEach(function(mqtt_element){
            mqtt_element.style.display = 'block';
            mqtt_type_checked();
        });
    }else{
        mqtt_uri.style.display = 'block';
        mqtt_no_advance.forEach(function(mqtt_element){
            mqtt_element.style.display = 'none';
        });
    }
}

function mqtt_validar(evt){

    evt.preventDefault();

    let mqtt_values = {advance:null,uri:null,ip:null,port:null,topic:null,type:null,user:null,password:null,app_layer:null,topic:null};

    mqtt_values.advance = mqtt_configuration.checked;
    mqtt_values.port = document.getElementById('mqtt_port').value;

    if(mqtt_values.port === ""){
        alert('Introduce a Port');
        return false;
    }

    mqtt_values.topic = document.getElementById('mqtt-topic').value;

    if(mqtt_values.topic === ""){
        alert('Introduce a Topic');
        return false;
    }

    if(mqtt_values.advance){

        mqtt_values.ip = document.getElementById('mqtt_ip').value;

        let regularEXPIP = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
        let resultIP = regularEXPIP.test(mqtt_values.ip);

        if(resultIP === false){
            alert('Introduce a valid IP address');
            return false;
        }

        if(mqtt_check_public.checked){
            mqtt_values.type = 'public';
        }else{
            mqtt_values.type = 'private'
            mqtt_values.password = document.getElementById('mqtt-password').value;
            if(mqtt_values.password === ""){
                alert('Introduce a Password');
                return false;
            }
        }
        mqtt_values.user = document.getElementById('mqtt_user').value;
        if(mqtt_values.user === ""){
            alert('Introduce an User');
            return false;
        }
        mqtt_values.app_layer = document.getElementById('mqtt_app_layer').options[mqtt_app_layer.selectedIndex].value;
        if(mqtt_values.app_layer === 'null'){
            alert('Introduce an App layer');
            return false;
        }
        
    }else{

        mqtt_values.uri = document.getElementById('mqtt_uri').value;

        if(mqtt_values.uri === ""){
            alert('Introduce a URI');
            return false;
        }
        
    }

    send_mqtt_form(mqtt_values);

}

function send_mqtt_form(mqtt_values){

    req = new XMLHttpRequest();

    if(this.readyState==4 && this.status==200){

        let respuesta = this.responseText;
        
        if(respuesta === 'safe'){
            alert('Succesfully safe the MQTT Data');
        }else{
            alert('There is a mistake in the form');
        }
    }

    myObj_string = JSON.stringify(mqtt_values);
    console.log(myObj_string);

     req.open("POST", "/mqtt", true);
     req.send(myObj_string);
    
}

function cargarMQTT(){
    
    verifica_config();

    req = new XMLHttpRequest();

    req.onreadystatechange = function(){

        if(this.readyState==4 && this.status==200){

            let respuesta = JSON.parse(this.responseText);
            
            document.getElementById('mqtt_port').value = respuesta.port;
            document.getElementById('mqtt-topic').value = respuesta.topic;
            document.getElementById('mqtt_uri').value = respuesta.uri;

        }

    }

     req.open("GET", "/mqtt", true);
     req.send();

}

document.addEventListener('DOMContentLoaded',cargarMQTT);



