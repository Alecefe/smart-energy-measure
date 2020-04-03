function pulsos(){

    if(document.getElementById("tipo_med").value=="pulsos"){
        document.getElementById("med_pulse").style.display = "block";
        document.getElementById("med_rs485").style.display = "none";
    }
    if(document.getElementById("tipo_med").value=="rs485"||document.getElementById("tipo_med").value=="chino"){
        document.getElementById("med_rs485").style.display = "block";
        document.getElementById("med_pulse").style.display = "none";
    }
    if(document.getElementById("tipo_med").value=="enlace"){
        document.getElementById("med_rs485").style.display = "none";
        document.getElementById("med_pulse").style.display = "none";

    }
    
}