function $(arg){
     try{return document.getElementById(String(arg));}
     catch(error){console.log(error);}
 }

const formulario =$('login');
formulario.addEventListener('submit',validar);

function user_send(valor_campos){

     var req = new XMLHttpRequest();

     req.onreadystatechange = function(){
          if(this.readyState==4 && this.status==200){
               console.log(this.responseText);
               if(this.responseText == "valido"){
               location.href = "/app-mesh.html";
               }else{
                    alert('Usuario o contraseña invalido');
                    formulario.reset();
               }
          }
     }

     myObj_string = JSON.stringify(valor_campos);

     console.log(myObj_string);

     req.open("POST", "/login", true);
     req.send(myObj_string);
}

function validar(evt) {

     evt.preventDefault();

     let valor_campos = {"usuario" : null, "contrasena": null,"remember":null};

     for(i in valor_campos){
          if(i==='remember'){
               valor_campos[i] = $(i).checked;
               continue;
          }
          valor_campos[i] = $(i).value;
     }

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

     user_send(valor_campos);
}