var lastHtml = '';

$('.result .bookLink').click(function() {
    var d = $(this).parent('.result');
    var a = d.find('a.bookLink');
    var link = a.attr('href');
    
    if(link) {
        $('body').find('.currentResult').html(function(){
            if(lastHtml) {
                $(this).addClass('result');
                $(this).removeClass('currentResult');
                return lastHtml;
            }
        });

        d.addClass('currentResult');
        d.removeClass('result');
        lastHtml = d.html();
        d.html('');

        var parText = $('<p>', {'class': 'parText'});
        parText.html(MainWindow.getPage(link));

        var parNav = $('<p>', {'class': 'pageNav'});
        parNav.append($('<a>', {'text': 'الصفحة السابقة', 'href': link, 'class': 'prev'}).click(function() {
            var par = parText;
            var pTxt = MainWindow.getPrevPage($(this).attr('href'));
            MainWindow.updateNavgitionLinks($(this).attr('href'));
            
            par.html(pTxt);
            scroll(d.position().left, d.position().top);
        }));   
        
        parNav.append($('<a>', {'text': 'الصفحة التالية', 'href': link, 'class': 'next'}).click(function() {
            var par = parText;
            var nTxt = MainWindow.getNextPage($(this).attr('href'));
            MainWindow.updateNavgitionLinks($(this).attr('href'));
            
            par.html(nTxt);
            scroll(d.position().left, d.position().top);
        }));

        d.append(parNav);
        d.append($('<div>', {'class': 'clearDiv'}));
        d.append(parText);
        
        scroll(d.position().left, d.position().top);
    }
});

function updateLinks(nextUrl, prevUrl) {
    $('.next').attr('href', nextUrl);
    $('.prev').attr('href', prevUrl);
}