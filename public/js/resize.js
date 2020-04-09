var t;
var d = 200;
var timeout = false;

let setTableMin = false
let setViewMin = false

svgImage.width(viewPanel.width())
// logPanel.find('.panel-header').width(logPanel.width() - 40)

$(document).ready(() => updateViewDimensions())

function updateViewDimensions() {
    var w = Math.round( $('#svg-image').width() )
    var h = Math.round( $('#svg-image').height() )
    $('#view-dimensions').text( `${w} x ${h}` )
};


$("#view-panel").resize((e) => {
    let win = $(window)
    let offset = e.offset || 50

    viewPanel.width(viewPanel.width() + 100)
    viewPanel.css('left', win.width() - viewPanel.width() + ((e.offset) ? 50 : 0))
    viewPanel.width(viewPanel.width() - offset)

    // svgImage.width(svgView.width())

    if (logPanel[0].offsetWidth < logPanel[0].scrollWidth && !setTableMin) {
        $('#file-table').css('min-width', $('#file-table').width())
        setTableMin = true
    }

    if (viewPanel[0].offsetHeight < viewPanel[0].scrollHeight) {
        // svgImage.css('max-width', $('#svg-image').width())
        // svgView.css('max-width', $('#svg-view').width())
        console.log('setting')
    }

    if (viewHeader[0].offsetWidth < viewHeader[0].scrollWidth) {
        viewPanel.css('min-width', $('#svg-image').width())
    }
    
    // updateViewDimensions()

    logPanel.width(win.width() - viewPanel.width() - 75)
    // logPanel.find('.panel-header').width(logPanel.width() - 40)

    e.stopPropagation()
})

$(window).resize(() => {
    t = new Date();

    if (timeout === false) {
        timeout = true;
        setTimeout(resizeEnd, d);
    }
})

function resizeEnd() {
    // $('#svg-image').css('height', svgView.height())

    if (new Date() - t < d) {
        setTimeout(resizeEnd, d);

    } else {
        var event = $.Event('resize')
        event.offset = 100
        $("#view-panel").trigger(event)

        timeout = false;
    }               
}