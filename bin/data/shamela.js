var lastHtml = '';
var lastTop = 0;
var lastLeft = 0;
var baseUrl = resultWidget.baseUrl();
var clearBody = false;

function handleEvents() {
    $('.result p.resultText').click(function() {
        var d = $(this).parent('.result');
        var p = d.find('p.resultText');
        var bookID = p.attr('bookid');
		var resultID = p.attr('rid');

		lastHtml = $('body').html();
		lastTop = d.position().top, 
		lastLeft = d.position().left;
		
		var parText = $('<div>', {'class': 'parText'});
		resultWidget.openResult(bookID, resultID);
		
		parText.html(bookReader.homePage());

		var parNav = $('<div>', {'class': 'pageNav'});

		parNav.append($('<img>', {'alt': 'الصفحة السابقة', 'src': baseUrl+'/data/images/go-next.png', 'class': 'prev'}).click(function() {
			var pTxt = bookReader.prevPage();
			
			parText.html(pTxt);
			updateInfoBar();
		}));   
		
		parNav.append($('<img>', {'alt': 'عودة للنتائج', 'src': baseUrl+'/data/images/go-up.png', 'class': 'backToResults'}).click(function() {
			$('body').html(lastHtml);
			scroll(lastLeft, lastTop);
			
			resultWidget.showNavigationButton(true);
			handleEvents();
		}));
		
		parNav.append($('<img>', {'alt': 'الصفحة التالية', 'src': baseUrl+'/data/images/go-previous.png', 'class': 'next'}).click(function() {
			var nTxt = bookReader.nextPage();
			
			parText.html(nTxt);
			updateInfoBar();
		}));
		
		var bookNameText = '<span class="bName">' + /*'كتاب: ' +*/ bookReader.bookName() + '</span>';
		
		var locationBarText  = bookReader.currentPage() + ' / ' + bookReader.currentPart();
		
		var pageHead = $('<div>', {'class': 'pageHead'});
		var bookNameDiv = $('<div>', {'html': bookNameText, 'class': 'bookNameDiv'});
		var locationDiv = $('<div>', {'text': locationBarText, 'class': 'bLocation'});
		
		pageHead.append(bookNameDiv);
		pageHead.append(parNav);
		pageHead.append($('<div>', {'class': 'clearDiv'}));
		
		var r = $('<div>', {'class': 'currentResult'});
		r.append(pageHead);
		r.append(parText);
		r.append(locationDiv);
		
		$('body').html(r);
		
		resultWidget.showNavigationButton(false);
		scroll(0, 0);
    });
}

function updateLinks(nextUrl, prevUrl) {
    $('.next').attr('href', nextUrl);
    $('.prev').attr('href', prevUrl);
}

function updateInfoBar() {
    var infoBar = $('body').find('.bLocation');
    var infoText = bookReader.currentPage() + ' / ' + bookReader.currentPart() ;
                
    infoBar.text(infoText);
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
