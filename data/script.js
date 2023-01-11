//document.addEventListener('DOMContentLoaded', receiveState());
document.addEventListener('DOMContentLoaded', ()=>{
    
});
//DADOS FORMS 
//document.addEventListener('submit',sendJSON())

const addtagEndpoint= '/addtag';
const formEl= document.querySelector('.form');
const lastnfc= document.getElementById('lastNFC').va;

formEl.addEventListener('submit',(e)=>{
    e.preventDefault();
    const formData=new FormData(formEl);
    const data= Object.fromEntries(formData);
    console.log(data);
    postData (addtagEndpoint,data)
})



// Example POST method implementation:
async function postData(url = '', data = {}) {
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
    body: JSON.stringify(data) // body data type must match "Content-Type" header
    
    });
    console.log(response.json);
    return response.json(); // parses JSON response into native JavaScript objects
}


