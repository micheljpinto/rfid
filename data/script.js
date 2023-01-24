//document.addEventListener('DOMContentLoaded', receiveState());
//document.addEventListener('DOMContentLoaded', getData());
//DADOS FORMS 
//document.addEventListener('submit',sendJSON())

const addtagEndpoint= '/addtag';
const addtagEndpoint2= '/lastread';
//const addtagEndpoint="https://63bff7ffa177ed68abbc883c.mockapi.io/api/v1/tag";
//const addtagEndpoint2= "https://63bff7ffa177ed68abbc883c.mockapi.io/api/v1/lastread";
const formEl= document.querySelector('.form');
const button = document.getElementById("bt");

button.addEventListener("click",printSensors);

formEl.addEventListener('submit',(e)=>{
    e.preventDefault();
    const formData=new FormData(formEl);
    const dataForm= Object.fromEntries(formData);
    //console.log(data);
    postData (addtagEndpoint,dataForm) .then((data) => {
        console.log(data); // JSON data parsed by `data.json()` call
      });
      formEl.reset();
})


// Example POST method implementation:
async function postData(url, dataForm = {}) {
    // Default options are marked with *
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



// window.addEventListener ('load', function () {
//     setInterval (printSensors, 7000);
// }, false);



function printSensors(){
    fetch(addtagEndpoint2)
    .then(response => response.text())
    .then(text => document.getElementById("ultima_tag").innerText=text) 
} 


const bt_atuador= document.getElementById('open');
bt_atuador.addEventListener('click',()=>{
    fetch('/atuador');
})  