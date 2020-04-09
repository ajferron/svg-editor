$('#expand-components').on('click', () => {
    $('#expand-components').toggleClass('active')
    $('#expand-components').siblings('.collapsible-content').toggleClass('active')

    if ( svgWrapper.hasClass('max') ) {
        $("#expand-components .expand-arrow").attr("src","./icons/chevron-up.svg");

        svgWrapper.toggleClass('min max')
        svgImage.width(200)
        svgView.width(200)

        setTimeout(() => {
            updateViewDimensions()
        }, 500)

    } else {
        $("#expand-components .expand-arrow").attr("src","./icons/chevron-down.svg");

        svgImage.width(viewPanel.width())
        svgView.css('width', '100%');

        setTimeout(() => { 
            svgWrapper.toggleClass('min max') 
            updateViewDimensions()
        }, 500)
    }
})