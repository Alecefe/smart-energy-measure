function $(arg){
     try{return document.getElementById(String(arg));}
     catch(error){console.log(error);}
}

const modoModbus = document.querySelector('.custom-select');
const divBaudRate = document.querySelector('.modbus-baud-rate-div');
const divConversionFactor = document.querySelector('.modbus-conversion-factor-div');
const divInitialEnergy = document.querySelector('.modbus-initial-energy-div');
const divSlaveId = document.querySelector('.modbus-slave-id-div');
const formModbus = $('form-modbus');

divBaudRate.style.display = 'none';
divConversionFactor.style.display = 'none';
divInitialEnergy.style.display = 'none';
divSlaveId.style.display = 'none';

eventListeners();

function eventListeners(){
     modoModbus.addEventListener('change', revisarSelector);
     formModbus.addEventListener('submit', validarModbus);

}

function revisarSelector(){
     
     selector = modoModbus.value;
     switch(selector){
          case 'enlace': 
               divBaudRate.style.display = 'none';
               divConversionFactor.style.display = 'none';
               divInitialEnergy.style.display = 'none';
               divSlaveId.style.display = 'none';
               break;
          
          case 'logo':
               divBaudRate.style.display = 'block';
               divConversionFactor.style.display = 'none';
               divInitialEnergy.style.display = 'none';
               divSlaveId.style.display = 'block';
               break;

          case 'pulsos':
               divBaudRate.style.display = 'block';
               divConversionFactor.style.display = 'block';
               divInitialEnergy.style.display = 'block';
               divSlaveId.style.display = 'block';
               break;
          case 'standard-rs485':
               divBaudRate.style.display = 'block';
               divSlaveId.style.display = 'block';
               divInitialEnergy.style.display = 'none';
               divConversionFactor.style.display = 'none';
               break;

     }

}

function validarModbus(e){

     e.preventDefault();

     let modbusValues = {type:null, baudrate:null, convfac:null, iniene:null, slaveid:null};

     modbusValues.type = modoModbus.value;

     switch (modoModbus.value){
          case 'enlace':
               break;
          
          case 'logo':

               modbusValues.type = modoModbus.value;
               if($('modbus-baud-rate').value === ''){
                    alert('Introduce a valid baud rate');
                    return false;
               }else{modbusValues.baudrate = $('modbus-baud-rate').value;}

               if($('modbus-slave-id').value === ''){
                    alert('Introduce a valid slave ID');
                    return false;
               }else{modbusValues.slaveid = $('modbus-slave-id').value;}
               break;
          
          case 'pulsos':

               if($('modbus-baud-rate').value === ''){
                    alert('Introduce a valid baud rate');
                    return false;
               }else{modbusValues.baudrate = $('modbus-baud-rate').value;}
          
               if($('modbus-conversion-factor').value === ''){
                    alert('Introduce a valid conversion factor');
                    return false;
               }else{modbusValues.convfac = $('modbus-conversion-factor').value;}
          
               if($('modbus-initial-energy').value === ''){
                    alert('Introduce a valid initial energy');
                    return false;
               }else{modbusValues.iniene = $('modbus-initial-energy').value;}
          
               if($('modbus-slave-id').value === ''){
                    alert('Introduce a valid slave ID');
                    return false;
               }else{modbusValues.slaveid = $('modbus-slave-id').value;}

               break;

          case 'standard-RS485':

               if($('modbus-baud-rate').value === ''){
                    alert('Introduce a valid baud rate');
                    return false;
               }else{modbusValues.baudrate = $('modbus-baud-rate').value;}

               if($('modbus-slave-id').value === ''){
                    alert('Introduce a valid slave ID');
                    return false;
               }else{modbusValues.slaveid = $('modbus-slave-id').value;}

               break;

     }
     
     send_modbus_form(modbusValues);
}

function send_modbus_form(modbus_values){

     var req = new XMLHttpRequest();

     if(this.readyState==4 && this.status==200){

          let respuesta = this.responseText;
          
          if(respuesta === 'safe'){
               alert('Succesfully safe the MODBUS Data');
          }else{
               alert('There is a mistake in the form');
          }
     }

     myObj_string = JSON.stringify(modbus_values);
     console.log(myObj_string);
  
     req.open("POST", "/modbus", true);
     req.send(myObj_string);
}

function cargarModbus(){
 
     req = new XMLHttpRequest();
 
     req.onreadystatechange = function(){
 
          if(this.readyState==4 && this.status==200){
 
               let respuesta = JSON.parse(this.responseText);
               
               document.querySelector('.custom-select').value = respuesta.type;
               $('modbus-baud-rate').value = respuesta.baudrate;
               $('modbus-conversion-factor').value = respuesta.convfac;
               $('modbus-initial-energy').value = respuesta.iniene;
               $('modbus-slave-id').value = respuesta.slaveid;

          }
 
     }
 
     req.open("GET", "/modbus", true);
     req.send();
 
}

document.addEventListener('DOMContentLoaded',cargarModbus);