var lastHtml = '';
var lastTop = 0;
var lastLeft = 0;
var clearBody = false;

function setStatusText(str) {
    $('.statusDiv').text(str);
}

function addResult(str) {
    $('body').append($(str));
}

function searchStarted() {
    $('body').html($('<p>', {text: "جاري البحث...", 'class': 'statusDiv'}));
}

function searchFinnished() {
    $('body').html('');
}

function fetechStarted() {
    if(clearBody == true) {
        $('body').html('');
    } else {
        clearBody = true;
    }
}

function setSearchTime(time) {
/*
    var sec = time / 1000;
    var str = 'ثم البحث خلال ' + sec + ' ثانية';
    
    $('body').html($('<p>', {'text': str, 'class': 'statusDiv'}));
    clearBody = false;
*/
}

function noResultFound() {
    var str = 'لم يتم العثور على ما يطابق بحثك';
    $('body').html($('<p>', {'text': str, 'class': 'statusDiv'}).css('background-color', '#FBDCDC'));
}

function searchException(text, desc) {
    var str =   'حدث خطأ أثناء البحث: ' + 
                '<strong>' +
                text +
                '</strong>';
    if(desc) {
        str +=  '<br />' + desc;
    }
    
    $('body').html($('<p>', {'html': str, 'class': 'statusDiv'}).css('background-color', '#FBDCDC'));
}
