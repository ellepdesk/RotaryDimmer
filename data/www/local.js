var idleTime = 5000; // immediate execution
var idleTimeMax = 5000;
var idleInterval = 1000;    
// (function worker() {
//     $.getJSON({
//         url:"values", 
//         success: function(data) {
//             $.each(data, function(k, v){
//                 $('#'+k).val(v);
//             })},
//         complete: function() {
//         // Schedule the next request when the current one's complete
//             setTimeout(worker, 2000);
//         }
//     });
//   })();



$('#intensity').change(function(){
$.ajax({
    url : 'setvalues',
    data : JSON.stringify({"intensity": $('#intensity').val()}),
    type : 'PATCH',
    contentType : 'application/json',
    success: function(data) {
        idleTime = 0;
        },
    });
});

$(document).ready(function () {
    setInterval(timerIncrement, idleInterval); //ms
});

function timerIncrement() {
    idleTime = idleTime + idleInterval;
    if (idleTime >= idleTimeMax) { 
        $.getJSON({
            url:"values", 
            success: function(data) {
                $.each(data, function(k, v){
                    $('#'+k).val(v);
                })
            },
        });
        idleTime = 0;
    }
}