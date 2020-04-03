
function $(arg){
     return document.getElementById(String(arg));
 }

function reset_form(){
     $('usuario').value = null;
     $('contrasena').value = null;
}
 
function user_review(){

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
      
     for( i in myObj){
          myObj[i] = $(i).value;
     }

     myObj_string = JSON.stringify(myObj);

     req.open("POST", "/login", true);
     req.send(myObj_string);
}