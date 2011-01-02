var lastHtml = '';
function handleEvents() {
    $('.result .bookLink').click(function() {
        var d = $(this).parent('.result');
        var a = d.find('a.bookLink');
        var link = a.attr('href');
        
        if(link) {
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

            d.addClass('currentResult');
            d.removeClass('result');
            lastHtml = d.html();
            d.html('');

            var parText = $('<p>', {'class': 'parText'});
            parText.html(resultWidget.getPage(link));

            var parNav = $('<p>', {'class': 'pageNav'});
            parNav.append($('<a>', {'text': 'الصفحة السابقة', 'href': link, 'class': 'prev'}).click(function() {
                var par = parText;
                var pTxt = resultWidget.getPage($(this).attr('href'));
                resultWidget.updateNavgitionLinks($(this).attr('href'));
                
                par.html(pTxt);
                scroll(d.position().left, d.position().top);
            }));   
            
            parNav.append($('<a>', {'text': 'الصفحة التالية', 'href': link, 'class': 'next'}).click(function() {
                var par = parText;
                var nTxt = resultWidget.getPage($(this).attr('href'));
                resultWidget.updateNavgitionLinks($(this).attr('href'));
                
                par.html(nTxt);
                scroll(d.position().left, d.position().top);
            }));

            d.append(parNav);
            d.append($('<div>', {'class': 'clearDiv'}));
            d.append(parText);
            
            resultWidget.updateNavgitionLinks(link);
            scroll(d.position().left, d.position().top);
        }
    });
}

function updateLinks(nextUrl, prevUrl) {
    $('.next').attr('href', nextUrl);
    $('.prev').attr('href', prevUrl);
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
