$(document).on('table-loaded', () => {
    $(".dd-selected").remove()
    $(".dd-options").remove()

    $(".dropdown").each((i, e) => {
        var wrapper = $(e)

        var select = wrapper.children('select')

        wrapper.append('<div class="dd-selected"> </div>')
        wrapper.append('<div class="dd-options dd-hide"> </div>')

        var fselect = wrapper.children('.dd-selected')
        var options = wrapper.children('.dd-options')

        fselect.text($(select).children('option:selected').text())

        select.children().each((i, e) => {
            options.append(`<div>${e.innerHTML}</div>`)
        })

        options.children().last().addClass('dd-last')

        options.children().on('click', (e) => {
            options.children().removeClass('last-selected')
            options.children('.dd-selected-opt').addClass('last-selected')

            options.children().each((i, opt) => {
                if (opt.innerHTML == e.target.innerHTML) {
                    select[0].selectedIndex = i;
                    fselect.text(opt.innerHTML)
                    options.children().removeClass('dd-selected-opt')
                    $(opt).addClass('dd-selected-opt')
                }
            })

            $(e.target).closest('.dropdown').trigger('selection')

            options.click()
        })
        
        fselect.on('click', (e) => {
            e.stopPropagation();
            closeAllSelect(e.target);
            options.toggleClass('dd-hide')
            fselect.toggleClass("dd-arrow-active")
        })
    })
})

function closeAllSelect(trigger) {
    $('.dd-selected').each((i, e) => {
        if (e != trigger) {
            $(e).removeClass('dd-arrow-active')
            $(e).siblings('.dd-options').addClass('dd-hide')
        }
    })
}

$(document).on('click', closeAllSelect)


$('#shape-dropdown').on('selection', (e) => {
    let selected = $(e.target).children('.dd-selected')

    $('.attr-form').removeClass('active')

    switch ( selected.text() ) {
        case "Type":
            break;

        case "Rectangle":
            $('#rect-attrs').addClass('active')
            break;

        case "Circle":
            $('#circle-attrs').addClass('active')
            break;
            
        case "Path":
            $('#path-attrs').addClass('active')
            break;    
    }

    centerDialog( $(e.target).closest('.dialog-box') )
});
