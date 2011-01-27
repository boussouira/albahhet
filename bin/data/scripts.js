var lastHtml = '';
var lastTop = 0;
var lastLeft = 0;
var baseUrl = resultWidget.baseUrl();
var clearBody = false;

function handleEvents() {
    $('.result .bookLink').click(function() {
        var d = $(this).parent('.result');
        var a = d.find('a.bookLink');
        var link = a.attr('href');
        
        if(link) {
            lastHtml = $('body').html();
            lastTop = d.position().top, 
            lastLeft = d.position().left;
            
            var parText = $('<p>', {'class': 'parText'});
            parText.html(resultWidget.getPage(link));

            var parNav = $('<p>', {'class': 'pageNav'});

            parNav.append($('<img>', {'alt': 'الصفحة السابقة', 'src': baseUrl+'/data/images/go-next.png', 'href': link, 'class': 'prev'}).click(function() {
                var pTxt = resultWidget.getPage($(this).attr('href'));
                resultWidget.updateNavgitionLinks($(this).attr('href'));
                
                parText.html(pTxt);
            }));   
            
            parNav.append($('<img>', {'alt': 'عودة للنتائج', 'src': baseUrl+'/data/images/go-up.png', 'class': 'backToResults'}).click(function() {
                $('body').html(lastHtml);
                scroll(lastLeft, lastTop);
                
                resultWidget.showNavigationButton(true);
                handleEvents();
            }));
            
            parNav.append($('<img>', {'alt': 'الصفحة التالية', 'src': baseUrl+'/data/images/go-previous.png', 'href': link, 'class': 'next'}).click(function() {
                var nTxt = resultWidget.getPage($(this).attr('href'));
                resultWidget.updateNavgitionLinks($(this).attr('href'));
                
                parText.html(nTxt);
            }));
            
            var infoText = '<span class="bName">' + /*'كتاب: ' +*/ resultWidget.currentBookName() + '</span>';
            infoText += '<span class="bLocation">' + 'الصفحة: ' 
                        + resultWidget.currentPage() + ' - الجزء: ' 
                        + resultWidget.currentPart() + '</span>';
            var infoBar = $('<div>', {'html': infoText, 'class': 'page_info'});
            infoBar.append($('<div>', {'class': 'clearDiv'}));
            
            var r = $('<div>', {'class': 'currentResult'});
            r.append(infoBar);
            r.append(parNav);
            r.append(parText);
            r.append($('<div>', {'class': 'clearDiv'}));
            $('body').html(r);
            resultWidget.updateNavgitionLinks(link);
            resultWidget.showNavigationButton(false);
        }
    });
}

function updateLinks(nextUrl, prevUrl) {
    $('.next').attr('href', nextUrl);
    $('.prev').attr('href', prevUrl);
}

function updateInfoBar(bookName, page, part) {
    var infoBar = $('body').find('.page_info');
    var infoText = '<span class="bName">' /*+ 'كتاب: ' */
                + bookName + '</span>'
                + '<span class="bLocation">' + 'الصفحة: ' 
                + page + ' - الجزء: ' 
                + part + '</span>';
                
    infoBar.html(infoText);
    infoBar.append($('<div>', {'class': 'clearDiv'}));
}

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
    var sec = time / 1000;
    var str = 'ثم البحث خلال ' + sec + ' ثانية';
    
    $('body').html($('<p>', {'text': str, 'class': 'statusDiv'}));
    clearBody = false;
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

$(document).ready(handleEvents());
