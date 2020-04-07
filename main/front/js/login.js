
var id_campos = ['usuario','contrasena'];

function $(arg){
     try{return document.getElementById(String(arg));}
     catch(error){console.log(error);}
 }

function reset_form(){
     for (i in id_campos){
          $(id_campos[i]).value = '';
     }
}

function user_send(){

     var req = new XMLHttpRequest();

     myObj = {'usuario': null, 'contrasena':null}

     req.onreadystatechange = function(){
          if(this.readyState==4 && this.status==200){
               console.log(this.responseText);
               if(this.responseText == "valido"){
               location.href = "/paginas/meshconf.html";
               }else{
                    alert('Usuario o contraseña invalido');
                    reset_form();
               }
          }
     }
      
     for(i in myObj){
          myObj[i] = $(i).value;
     }

     myObj_string = JSON.stringify(myObj);

     req.open("POST", "/login", true);
     req.send(myObj_string);
}

function validar() {
     var valor_campos = {"usuario" : null, "contrasena": null,};

     valor_campos.usuario=$('usuario').value;
     valor_campos.contrasena=$('contrasena').value;

     for(i in valor_campos){
          if (valor_campos[i]===''){
               alert('Usuario y contraseña requeridos');
               return false;
          }
          if (String(valor_campos[i]).length>15){
               alert('El usuario o contraseña no debe contener más de 15 caracteres');
               return false;
          }
     }
     user_send();
}