let logPanel = $("#file-panel")
let viewPanel = $("#view-panel")
let viewHeader = $('#view-panel .panel-header')
let svgView = $('#svg-view')
let svgImage = $("#view-panel #svg-view img.max")
let saveButton = $('button#save')
let attrOwner = null
let loaded = false

let active = null

let changes
resetChanges()

function resetChanges() {
    changes = {
        filepath: null,
        title: null,
        description: null,
        components: {},
        active: false
    }
}

function addComponentChange(component) {
    var change = {}
    var name = component.find('.component-name').first().text()
    console.log(name)

    change.type = name.split(' ')[0]
    change.idx = name.split(' ')[1] - 1
    change.attributes = []

    component.find('.attr-wrapper').each((i,wrap) => {
        change.attributes.push({
            name: $(wrap).find('.attr-name').text().replace(':', ''),
            value: $(wrap).find('.attr-value').val()
        })
    })

    changes.components[name] = change
}

function listOption(json) {
    var fname = json.filepath.split('/')[1]

    return new Option(fname, json.filepath)
}

function tableRow(summary) {
    var row = $( $('#file-row-template').html() )

    row.find('a').attr('href', summary.filepath)
    row.find('#thumb img').attr('src', summary.filepath)
    row.find('#filename a').text(summary.filepath.split('/')[1])
    row.find('#filesize').text(`${summary.size} KB`)
    row.find('#num-rects').text(`Rectangles: ${summary.numRects}`)
    row.find('#num-circles').text(`Circles: ${summary.numCircles}`)
    row.find('#num-paths').text(`Paths: ${summary.numPaths}`)
    row.find('#num-groups').text(`Groups: ${summary.numGroups}`)

    return row;
}

function svgComponentRow(svg, i) {
    var component = $( $('#component-row-template').html() )
    var reqAttrList = component.find('.component-attrs .required-attrs')
    var otherAttrList = component.find('.component-attrs .other-attrs')

    component.find('.component-name').text(`SVG`)
    component.addClass('rect')

    reqAttrList.css('display', 'none')

    svg.otherAttributes.forEach(attr => { otherAttrList.append( attrElement(attr) ) })

    return component;
}

function rectComponentRow(rect, i, parent) {
    var component = $( $('#component-row-template').html() )
    var reqAttrList = component.find('.component-attrs .required-attrs')
    var otherAttrList = component.find('.component-attrs .other-attrs')

    if ( i < 0 ) i = $('#component-table .rect-components').children().length

    component.find('.component-name').text(`Rectangle ${i+1}`)
    component.addClass('rect')

    reqAttrList.append( attrElement({name: 'x', value: rect.x}) )
    reqAttrList.append( attrElement({name: 'y', value: rect.y}) )
    reqAttrList.append( attrElement({name: 'w', value: rect.w}) )
    reqAttrList.append( attrElement({name: 'h', value: rect.h}) )

    rect.otherAttributes.forEach(attr => { otherAttrList.append( attrElement(attr) ) })

    if ( parent ) component.find('.field-wrapper').addClass('child')

    return component;
}

function circleComponentRow(circle, i, parent) {
    var component = $( $('#component-row-template').html() )
    var reqAttrList = component.find('.component-attrs .required-attrs')
    var otherAttrList = component.find('.component-attrs .other-attrs')

    if ( i < 0 ) i = $('#component-table .circle-components').children().length

    component.find('.component-name').text(`Circle ${i+1}`)
    component.addClass('circle')

    reqAttrList.append( attrElement({name: 'cx', value: circle.cx}) )
    reqAttrList.append( attrElement({name: 'cy', value: circle.cy}) )
    reqAttrList.append( attrElement({name: 'r', value: circle.r}) )

    circle.otherAttributes.forEach(attr => { otherAttrList.append( attrElement(attr) ) })

    if ( parent ) component.find('.field-wrapper').addClass('child')

    return component;
}

function pathComponentRow(path, i, parent) {
    var component = $( $('#component-row-template').html() )
    var reqAttrList = component.find('.component-attrs .required-attrs')
    var otherAttrList = component.find('.component-attrs .other-attrs')

    component.find('.component-name').text(`Path ${i+1}`)
    component.addClass('path')

    reqAttrList.append( attrElement({name: 'd', value: path.d}) )

    path.otherAttributes.forEach(attr => { otherAttrList.append( attrElement(attr).addClass('path') ) })

    if ( parent ) component.find('.field-wrapper').addClass('child')

    return component;
}

function groupComponentRow(group, i, parent) {
    var component = $( $('#component-row-template').html() )
    var reqAttrList = component.find('.component-attrs .required-attrs')
    var otherAttrList = component.find('.component-attrs .other-attrs')

    component.find('.component-name').text(`Group ${i+1}`)
    component.find('.field-wrapper').addClass('parent')
    component.addClass('expandable')
    component.addClass('group')

    reqAttrList.css('display', 'none')

    group.otherAttributes.forEach(attr => { otherAttrList.append( attrElement(attr) ) })

    if ( parent ) component.find('.field-wrapper').addClass('child')

    group.rectangles.forEach((rect, i) => { component.append( rectComponentRow(rect, i, component) ) })
    group.circles.forEach((circle, i) => { component.append( circleComponentRow(circle, i, component) ) })
    group.paths.forEach((path, i) => { component.append( pathComponentRow(path, i, component) ) })
    group.groups.forEach((group, i) => { component.append( groupComponentRow(group, i, component) ) })
    
    return component;
}

function attrElement(attr) {
    var attribute = $( $('#attribute-template').html() )

    if (attr.name == 'd') attribute.addClass('path-attr')

    attribute.find('.attr-name').text(attr.name + ":")
    attribute.find('.attr-value').val(attr.value)

    return attribute;
}

$(document).ready(function() {
    $("#view-panel").resizable({
            handles: "w",
            minWidth: 520
        })

    $('#component-table').find('.attr-value').trigger('keyup', [])

    $('#overlay').removeClass('enabled')

    $('#svg-image').css('max-height', svgView.height())
    
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/table_data',

        success: function (summaries) {
            summaries.forEach(summary => {
                $('#file-table').append( tableRow(summary) )
                $("#file-list").append( listOption(summary) );
            })

            console.log("[!] Got SVG summary data from server")
            $(document).trigger('table-loaded')
        },

        fail: function(error) {
            console.error("[!] Failed to get SVG summary data from server " + error)
        }
    })
})



$('#upload-dialog form').submit((e) => {
    e.preventDefault()

    var formData = new FormData();
    formData.append('image', $('#upload-dialog input#svg-file')[0].files[0]); 

    $.ajax({
        url: '/upload',
        type: 'POST',
        data: formData,
        contentType: false, 
        processData: false,

        success: function (res) {
            if (res.summary != null) {
                $('#file-table').append( tableRow(res.summary) )
                $("#file-list").append( listOption(res.summary) )
            }

            alert(res.msg)

            $(document).trigger('table-loaded')
        },

        fail: function(error) { alert(error) }
    });
    
    disableDialog( $(e.target).closest('.dialog-box') );
})



$('#view-panel textarea').on('input', (e) => {
    var textarea = $(e.target)[0]

    if (textarea.id == 'title-field') 
        changes.title = $('#title-field').val()
    else
        changes.description = $('#desc-field').val()

    saveButton.addClass('enabled')
})



$('#file-dropdown').on('selection', (e) => {
    let selected = $(e.target).children('.dd-selected')

    if ( $('#save').hasClass('enabled') )
        if ( !confirm('Discard changes to SVG?') )
            return;
        
    resetChanges()

    changes.filepath = `./uploads/${$('#file-dropdown .dd-selected').text()}`

    $('#component-table .component-wrapper').remove()

    if (selected.text() == 'SVG Files') {
        $('#svg-image').attr('src', '')
        $('#svg-data').addClass('disabled')
        $('#title-field').val('')
        $('#desc-field').val('')
        active = null;

    } else {
        $('#svg-image').attr('src', `uploads/${selected.text()}`)
        $('#svg-data').removeClass('disabled')
        active = selected;

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/svg_data',
            data: {fpath : selected.text()},

            success: function (svg) { 
                $('#title-field').val(svg.title)
                $('#desc-field').val(svg.description)
                
                $('#component-table .svg-component').append( svgComponentRow(svg, 0, null) )
                
                svg.rectangles.forEach((rect, i) => { $('#component-table .rect-components').append( rectComponentRow(rect, i, null) ) })
                svg.circles.forEach((circle, i) => { $('#component-table .circle-components').append( circleComponentRow(circle, i, null) ) })
                svg.paths.forEach((path, i) => { $('#component-table .path-components').append( pathComponentRow(path, i, null) ) })
                svg.groups.forEach((group, i) => { $('#component-table .group-components').append( groupComponentRow(group, i, null) ) })
                
                $('#component-table').find('.attr-value').trigger('keyup', [])
                changes.active = true

                saveButton.removeClass('enabled')
            },
            fail: function (error) { console.error("[!] Failed to get SVG summary data from server " + error) }
        });
    }
});




$('#add-attribute-dialog').submit((e) => {
    var component = attrOwner.closest('.component-attrs')
    var nm = $('#new-attr-name').val()
    var val = $('#new-attr-value').val()

    var newAttr = attrElement({name: nm, value: val});

    component.find('.other-attrs').append( newAttr )
    
    if ( changes.active ) addComponentChange(component.closest('.component-wrapper'))
    
    changes.active = false
    $('#component-table').find('.attr-value').trigger('keyup')
    changes.active = true

    disableDialog( $(e.target).closest('.dialog-box') );

    saveButton.addClass('enabled')

    e.preventDefault()
    attrOwner = null
})



$('#component-table').on('keydown keyup', 'input.attr-value', (e, data) => {
    var input = $(e.target)

    if ( changes.active ) addComponentChange(input.closest('.component-wrapper'))

    if ( (data || true) ) saveButton.addClass('enabled')

    if (input.prev().text() != 'd:')
        input.width((input.val().length + 1) * 9);
})

$('#component-table').on('click', '.add-attribute', (e) => {
    attrOwner = $(e.target)

    $('#add-attribute-dialog').find('input').val('')
    $('#view-components-dialog').css('pointer-events', 'none');

    enableDialog( $('#add-attribute-dialog') )

    e.stopPropagation()
})



$('#component-table').on('keydown keyup', '.rect-components .scale-input', (e) => {
    $('.rect-components .scale-input').val( $(e.target).val() )

    $('#save').addClass('enabled')
})

$('#component-table').on('keydown keyup', '.circle-components .scale-input', (e) => {
    $('.circle-components .scale-input').val( $(e.target).val() )

    $('#save').addClass('enabled')
})



$('#add-component').on('click', (e) => {
    $('#shape-dropdown .dd-options').children(':first').click()
    $('#add-component-dialog').find('input').val('')

    enableDialog( $('#add-component-dialog') )
    e.stopPropagation()
})

$('#view-components').on('click', (e) => {
    enableDialog( $('#view-components-dialog') )

    e.stopPropagation()
})

$('#add-component-dialog form').submit((e) => {
    var dialog = $(e.target).closest('.dialog-box')
    var type = $('#shape-dropdown').children('.dd-selected').text()
    var shape;

    switch ( type ) {
        case "Rectangle":
            shape = {
                x: $('#x-attr').val(),
                y: $('#y-attr').val(),
                w: $('#w-attr').val(),
                h: $('#h-attr').val(),
                otherAttributes: []
            }
            break;

        case "Circle":
            shape = {
                cx: $('#cx-attr').val(),
                cy: $('#cy-attr').val(),
                r: $('#r-attr').val(),
                otherAttributes: []
            }
            break;
    }

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/add_shape',
        data: {
            shape: shape,
            fpath: `uploads/${active.text()}`,
            type: type
        },

        success: function (res) {
            if (res.success) {
                console.log("[!] Added shape to file on server")
                alert('Component added successfully!')

                switch ( type ) {
                    case "Rectangle":
                        $('#component-table .rect-components').append( rectComponentRow(shape, -1, null) )
                        break;
            
                    case "Circle":
                        $('#component-table .circle-components').append( circleComponentRow(shape, -1, null) )
                        break;
                }

                location.reload();

            } else {
                console.error("[!] Failed to add shape to file on server")
                alert('Could not add component!')
            }
        },

        fail: function(error) {
            console.error("[!] Failed to add shape to file on server" + error)
        }
    })

    
    disableDialog(dialog);
    e.preventDefault()
})



$('#view-panel #close').on('click', (e) => {
    e.preventDefault()

    if ( $('#save').hasClass('enabled') )
        if ( !confirm('Discard changes to SVG?') ) {
            resetChanges()
            return;
        }

    $('#file-dropdown .dd-options').children().first().click()
})

$('#create-svg').on('click', (e) => {
    $('#new-file-dialog input').val('')
    enableDialog( $('#new-file-dialog') )
})

$('#new-file-dialog form').submit((e) => {
    var filename = $('#new-filename').val();

    e.preventDefault()

    if (filename == '') return

    if ( !filename.includes('.svg') ) 
        filename = filename.concat('.svg')
        
    var filepath = `uploads/${filename}`

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/create_file',
        data: {fpath : filepath},

        success: function (res) { 
            if ( res.json ) {
                res.json.filepath = filepath

                $('#file-table').append( tableRow(res.json) )
                $("#file-list").append( listOption(res.json) )
                $(document).trigger('table-loaded')

                active = $("#file-dropdown .dd-options").children().last()

                active.click()
            }

            alert(res.msg)
        },
        fail: function (error) { 
            console.error("[!] Failed to create SVG file on the server " + error) 
        }
    });

    disableDialog( $('#new-file-dialog') )
})



$('#save').on('click', (e) => {
    console.log(changes)
    e.preventDefault()

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/save_changes',
        data: {changes: JSON.stringify(changes)},

        success: function (res) { 

            alert(res.msg)
        },

        fail: function (error) { 
            console.error("[!] Failed to create SVG file on the server " + error) 
        }
    });

    $('#save').removeClass('enabled')
})

$('#component-table').on('click', '.parent .component-name', (e) => {
    // var component = $(e.target).closest('.component-wrapper.expandable')
    // component.toggleClass('open-group')
    // e.preventDefault()
})
