function $(arg){
     try{return document.getElementById(String(arg));}
     catch(error){console.log(error);}
}

const formUserLog = $('form-user-log');

eventListeners();

function eventListeners(){
     
     formUserLog.addEventListener('submit', validarUserLog);
}

function validarUserLog(e){
     
     e.preventDefault();

     let userLogValues = {user:null, pass:null};

     if($('net-login-user').value===''){
          alert('Introduce an Username');
          return false;
     }else if($('net-new-login-user').value===''){
          alert('Confirm your username');
          return false;
     }else if($('net-login-user').value!=$('net-new-login-user').value){
          alert('Username doesn\'t match');
          return false;
     }else{
          userLogValues.user = $('net-login-user').value;
     }

     if($('net-login-pass').value===''){
          alert('Introduce an password');
          return false;
     }else if($('net-new-login-pass').value===''){
          alert('Confirm your password');
          return false;
     }else if($('net-login-pass').value!=$('net-new-login-pass').value){
          alert('Password doesn\'t match');
          return false;
     }else{
          userLogValues.user = $('net-login-pass').value;
     }

     send_userlog_form(userLogValues);
}

function send_userlog_form(userLogValues){

     var req = new XMLHttpRequest();

     if(this.readyState==4 && this.status==200){

          let respuesta = this.responseText;
          
          if(respuesta === 'safe'){
               alert('Succesfully safe the new user login Data');
          }else{
               alert('There is a mistake in the form');
          }
     }

     myObj_string = JSON.stringify(userLogValues);
     console.log(myObj_string);
  
     req.open("POST", "/userlog", true);
     req.send(myObj_string);
}

function cargarUserLog(){
 
     req = new XMLHttpRequest();
 
     req.onreadystatechange = function(){
 
          if(this.readyState==4 && this.status==200){
 
               let respuesta = JSON.parse(this.responseText);
               
               $('net-login-user').value = respuesta.user;
               $('net-login-pass').value = respuesta.password;

          }
 
     }
 
     req.open("GET", "/userlog", true);
     req.send();
 
}

document.addEventListener('DOMContentLoaded',cargarUserLog);