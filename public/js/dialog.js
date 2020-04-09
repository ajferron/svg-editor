let overlay = $('#overlay')

function enableDialog(dialog) {
    if (overlay && dialog) {
        overlay.addClass('enabled')
        dialog.addClass('enabled')

        centerDialog(dialog)
    }
}

function disableDialog(dialog) {
    dialog.removeClass('enabled')

    if ( dialog[0].id != "add-attribute-dialog" ) {
        overlay.removeClass('enabled')
    } else {

        $('#view-components-dialog').css('pointer-events', 'all');
    }
}

function centerDialog(dialog) {
    dialog.offset({
        top: ($(window).height() - dialog.height()) / 2, 
        left: ($(window).width() - dialog.width()) / 2
    })
}

$(".dialog-box").each((i, e) => dragElement( e ) )

function dragElement(dialog) {
    var dX = 0, dY = 0, mX = 0, mY = 0;
    
    $(dialog).children('header')[0].onmousedown = dragMouseDown

    function dragMouseDown(e) {
        e = e || window.event;
        // e.preventDefault();
        
        mX = e.clientX;
        mY = e.clientY;
        document.onmouseup = closeDragElement;
        document.onmousemove = elementDrag;
    }

    function elementDrag(e) {
        e = e || window.event;
        // e.preventDefault();
        
        dX = mX - e.clientX;
        dY = mY - e.clientY;
        mX = e.clientX;
        mY = e.clientY;

        dialog.style.top = (dialog.offsetTop - dY) + "px";
        dialog.style.left = (dialog.offsetLeft - dX) + "px";
    }

    function closeDragElement() {
        document.onmouseup = document.onmousemove = null;
    }
}

$('.dialog-box .close-dialog').on('click', (e) => {
    disableDialog( $(e.target).closest('.dialog-box') );
})

$('#upload-dialog input:file').change((e) => {
    $('#upload-dialog label span').text( $(e.target).val().split('\\')[2] )
});


// $('#add-component-dialog .dd-options div').on('click', (e) => {
//     console.log($(e.target).text())
// })
