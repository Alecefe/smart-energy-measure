var orden = true;
        
        
function muestra_menu(){
    if(orden){
        document.getElementsByClassName("menu")[0].style.display = "block";
        orden=false;
    }
    else{
        document.getElementsByClassName("menu")[0].style.display = "none";
        orden=true;
    }
}
    