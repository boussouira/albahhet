var lastHtml = '';
var baseUrl = resultWidget.baseUrl();
function handleEvents() {
    $('.result .bookLink').click(function() {
        var d = $(this).parent('.result');
        var a = d.find('a.bookLink');
        var link = a.attr('href');
        
        if(link) {
            lastHtml = $('body').html();
            /*
            $('body').find('.currentResult').each(function(){
                if(lastHtml) {
                    $(this).addClass('result');
                    $(this).removeClass('currentResult');
                    $(this).html(lastHtml);
                    $(this).mouseenter(function() {
                        handleEvents();
                    });
                }
            });
            */
            /*
            d.addClass('currentResult');
            d.removeClass('result');
            lastHtml = d.html();
            d.html('');
            */
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
            //scroll(d.position().left, d.position().top);
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

function addResult(str) {
    $('body').append($(str));
}

function searchStarted() {
  $('body').html($('<p>', {text: "جاري البحث..."}));
}

function searchFinnished() {
  $('body').html('');
}

function fetechStarted() {
  $('body').html('');
}

$(document).ready(handleEvents());
$(document).ready($('body').fadeIn('slow'));
