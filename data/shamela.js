var baseUrl = resultWidget.baseUrl();
var clearBody = false;

function handleEvents() {
    $('.result p.resultText').click(function() {
        var d = $(this).parent('.result');
        var p = d.find('p.resultText');
        var bookID = p.attr('bookid');
        var resultID = p.attr('rid');

        resultWidget.openResult(bookID, resultID);
    });
}

function startReading() {
    var parText = $('<div>', {'class': 'parText'});

    parText.html(bookReader.homePage());
    var parNav = $('<span>', {'class': 'pageNav'});

    parNav.append($('<img>', {'alt': 'الصفحة السابقة',
                              'title': 'الصفحة السابقة',
                              'src': baseUrl+'/data/images/go-next2.png',
                              'class': 'prev'}).click(function() {
        var pTxt = bookReader.prevPage();

        parText.html(pTxt);
        updateInfoBar();
        scroll(0, 0);
    }));

    parNav.append($('<img>', {'alt': 'الصفحة التالية',
                              'title': 'الصفحة التالية',
                              'src': baseUrl+'/data/images/go-previous2.png',
                              'class': 'next'}).click(function() {
        var nTxt = bookReader.nextPage();

        parText.html(nTxt);
        updateInfoBar();
        scroll(0, 0);
    }));

    var bookNameText = bookReader.bookName();

    var locationBarText  = ' (' + 'الصفحة: ' + bookReader.currentPage() + ' - ' + 'الجزء: ' + bookReader.currentPart() + ')';

    var pageHead = $('<div>', {'class': 'pageHead'});
    var bookNameDiv = $('<span>', {'text': bookNameText, 'class': 'bookNameDiv'});
    var locationDiv = $('<span>', {'text': locationBarText, 'class': 'bLocation'});

    pageHead.append(bookNameDiv);
    pageHead.append(locationDiv);
    pageHead.append(parNav);
    pageHead.append($('<div>', {'class': 'clearDiv'}));

    var r = $('<div>', {'class': 'currentResult'});
    r.append(pageHead);
    r.append(parText);

    $('#searchResult').html(r);
}

function updateLinks(nextUrl, prevUrl) {
    $('.next').attr('href', nextUrl);
    $('.prev').attr('href', prevUrl);
}

function updateInfoBar() {
    var infoBar = $('#searchResult').find('.bLocation');
    var infoText = ' (' + 'الصفحة: ' + bookReader.currentPage() + ' - ' + 'الجزء: ' + bookReader.currentPart() + ')';

    infoBar.text(infoText);
}

function setStatusText(str) {
    $('.statusDiv').text(str);
}

function addResult(str) {
    $('#searchResult').append($(str));
}

function searchStarted() {
    $('#searchResult').html($('<p>', {text: "جاري البحث...", 'class': 'statusDiv'}));
}

function searchFinnished() {
    $('#searchResult').html('');
}

function fetechFinnished() {
    handleEvents();
    setupToolTip();
}

function setupToolTip() {
    $('.bookName').tooltip({
        track: true,
        delay: 800,
        showURL: false,
        showBody: " - ",
        extraClass: "pretty",
        fixPNG: true,
        opacity: 0.95,
        left: 0,
        positionLeft: true
    });
}

function fetechStarted() {
    if(clearBody == true) {
        $('#searchResult').html('');
    } else {
        clearBody = true;
    }
}

function setSearchTime(time) {
    var sec = time / 1000;
    var str = 'ثم البحث خلال ' + sec + ' ثانية';

    $('#searchResult').html($('<p>', {'text': str, 'class': 'statusDiv'}));
    clearBody = false;
}

function noResultFound() {
    var str = 'لم يتم العثور على ما يطابق بحثك';
    $('#searchResult').html($('<p>', {'text': str, 'class': 'statusDiv'}).css('background-color', '#FBDCDC'));
}

function searchException(text, desc) {
    var str =   'حدث خطأ أثناء البحث: ' +
                '<strong>' +
                text +
                '</strong>';
    if(desc) {
        str +=  '<br />' + desc;
    }

    $('#searchResult').html($('<p>', {'html': str, 'class': 'statusDiv'}).css('background-color', '#FBDCDC'));
}
