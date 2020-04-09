'use strict'

const ffi = require('ffi-napi');
const fs = require('fs');

const express = require("express");
const JavaScriptObfuscator = require('javascript-obfuscator');

const fileUpload = require('express-fileupload');
const path = require("path");
const app = express();

const portNum = process.argv[2];

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));


let SVGParser = ffi.Library('./libsvgparse.so', {
    'validateSVG': [ 'string', ['string'] ],
    'fileToJSON': [ 'string', ['string'] ],
    'fileToSummaryJSON': [ 'string', ['string'] ],
    'createSVGFile': [ 'string', ['string'] ],
    'addRectFromJSON': [ 'string', ['string', 'string'] ],
    'addCircleFromJSON': [ 'string', ['string', 'string'] ],
    'updateFileTitle': [ 'string', ['string', 'string'] ],
    'updateFileDesc': [ 'string', ['string', 'string'] ],
    'setAttributeInFile': [ 'string', ['string', 'string', 'int', 'string', 'string'] ]
});


function pathToSummaryJSON(fp) {
    var string = SVGParser.fileToSummaryJSON(fp);

    if ( string != "Invalid SVG" ) {
        var json = JSON.parse( string )

        json.filepath = fp
        json.size = Math.round( fs.statSync(fp)["size"]/1000 )

        return json;
    }

    return null;
}


// Send HTML at root, do not change
app.get('/',function(req,res){
    res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
    res.sendFile(path.join(__dirname + '/public/style.css'));
});

app.get('/js/*.js',function(req,res){
    fs.readFile(path.join(`${__dirname}/public${req.url}`), 'utf8', function(err, contents) {
        const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: false, controlFlowFlattening: true});
        res.contentType('application/javascript');
        res.send(minimizedContents._obfuscatedCode);
    });
});

app.post('/upload', function(req, res) {
    if (!req.files) {
        return res.status(400).send('No files were uploaded.');
    }

    let upload = req.files.image;
    let fpath = path.join('uploads/', upload.name)

    if ( fs.existsSync(fpath) ) return res.send({
        msg: 'Duplicate file upload!',
        summary: null
    })

    upload.mv(fpath, (err) => { 
        if (err) return res.status(500).send(err) 

        let json = pathToSummaryJSON(fpath)

        if ( !json ) {
            fs.unlink(fpath, () => {})

            return res.send({
                msg: 'Invalid SVG!',
                summary: null
            })
        }

        res.send({
            msg: 'Upload successful!',
            summary: json
        });
    });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
    fs.stat('uploads/' + req.params.name, function(err, stat) {
        if (err == null) {
            res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
        
        } else {
            console.log('Error in file downloading route: '+err);
            res.send('');
        }
    });
});



app.get('/css/*.css',(req,res) => {
    res.sendFile(path.join(`${__dirname}/public${req.url}`));
});

app.get('/icons/*.svg',(req,res) => {
    res.sendFile(path.join(`${__dirname}/public${req.url}`));
});

app.get('/table_data', (req , res) => {
    var summaries = []
    
    fs.readdir("./uploads/", (err, files) => {
        if (err) console.error("Could not open /uploads", err);
      
        files.forEach(file => {
            var fpath = path.join('./uploads/', file)
            var json = pathToSummaryJSON(fpath);

            if ( json ) summaries.push(json);
        });

        res.send(summaries)
    })
})


app.get('/svg_data', (req , res) => {
    var fpath = `uploads/${req.query.fpath}`
    var json = SVGParser.fileToJSON( fpath ).replace('\n', ' ')

    res.send( JSON.parse( json ) )
})


app.get('/create_file', (req , res) => {
    var fpath = req.query.fpath

    if ( fs.existsSync(fpath) ) return res.send({
        msg: 'A file already exists with that name!',
        json: null
    })

    var stat = SVGParser.createSVGFile( fpath )

    if ( stat == "Success")
        res.send({
            msg: 'Successfully created SVG file!',
            json: JSON.parse( SVGParser.fileToJSON(fpath) )
        })
    else
        res.send({
            msg: 'Failed to create SVG file!',
            json: null
        })
})


app.get('/add_shape', (req , res) => {
    var fpath = req.query.fpath
    var shape = JSON.stringify(req.query.shape)
    var status;

    switch (req.query.type) {
        case "Rectangle":
            status = SVGParser.addRectFromJSON(fpath, shape)
            break;

        case "Circle":
            status = SVGParser.addCircleFromJSON(fpath, shape)
            break;
    }

    res.send({success: status == "Success"})
})


app.get('/save_changes', (req , res) => {
    var c = JSON.parse(req.query.changes)

    if (c.title) SVGParser.updateFileTitle(c.filepath, c.title)
    if (c.description) SVGParser.updateFileDesc(c.filepath, c.description)

    for (var key in c.components) {
        var component = c.components[key];

        component.attributes.forEach(attr => {
            SVGParser.setAttributeInFile(c.filepath, component.type, component.idx || 0, attr.name, attr.value)
        })
    }

    res.send({msg: "Saved SVG changes to server!"})
})


app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
