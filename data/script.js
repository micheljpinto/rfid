
window.addEventListener('DOMContentLoaded', function () {
    const addtag= '/addtag';
    const rmtag= '/rmtag';
    const lastRead= '/lastread';
    const formEl=   document.querySelector('.form');
    const button = document.getElementById('bt');
    const bt_atuador= document.getElementById('open');
    const el = document.getElementById("checagem");

    button.addEventListener("click",printSensors);

    formEl.addEventListener('submit',(e)=>{
        e.preventDefault();  
        const formData=new FormData(formEl);
        const dataForm= Object.fromEntries(formData);
       
        postData (dataForm,returnLink) 
        .then((data) => {
            console.log(data); // JSON data parsed by `data.json()` call
          });
          formEl.reset();
    })

    bt_atuador.addEventListener('click',()=>{
        fetch('/atuador');
    })  

    function returnLink(){
        var link;
        if(el.checked){
            link=rmtag;
        }else{
            link=addtag;
        }
        return link;
    }

    function printSensors(){
        fetch(lastRead)
        .then(response => response.text())
        .then(text => document.getElementById("ultima_tag").innerText=text) 
    } 
    
})

async function postData(dataForm = {},callback) {
    // Default options are marked with *
    var url= callback();   

    const response = await fetch(url, {
    method: 'POST', // *GET, POST, PUT, DELETE, etc.
    mode: 'cors', // no-cors, *cors, same-origin
    cache: 'no-cache', // *default, no-cache, reload, force-cache, only-if-cached
    credentials: 'same-origin', // include, *same-origin, omit
    headers: {
        'Content-Type': 'application/json'
        // 'Content-Type': 'application/x-www-form-urlencoded',
        },
    redirect: 'follow', // manual, *follow, error
    referrerPolicy: 'no-referrer', // no-referrer, *no-referrer-when-downgrade, origin, origin-when-cross-origin, same-origin, strict-origin, strict-origin-when-cross-origin, unsafe-url
    body: JSON.stringify(dataForm) // body data type must match "Content-Type" header
    
    });

    let data= await response.json()
    console.log(data["status"]);
    //return response.json(); // parses JSON response into native JavaScript objects
}


