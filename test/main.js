let c = 10;
let d = "hello";

let click1 = false;
function sayhello(){
     for(let i = 0; i < 10; i++){
         console.log("hello world");
     }
     console.log("c is " + c);
     console.log("d is " + d);
}

function JustCallMe(){
   let heading = document.getElementById("heading");
   console.log("hello" + heading.innerHTML);
   heading.innerHTML = "Hello, JavaScript!";
   
}

let btn = document.getElementById("btn");
btn.addEventListener("click",JustCallMe);
console.log("JavaScript is running!");
console.log("This is a simple HTTP server example.");
console.log("You can access the server at http://127.0.0.1:8080");

let hiBtn = document.getElementById("hi-btn");

hiBtn.addEventListener("click", function() {

   let subHeading = document.getElementById("sub-heading");
   click1 = !click1; // Toggle clickCount
     if(click1){
          subHeading.innerHTML = "Hello, Hi Button!";
     }
     else{
          subHeading.innerHTML = "";
     }
});
