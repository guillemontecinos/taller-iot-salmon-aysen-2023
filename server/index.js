const express = require('express')
const bodyParser = require('body-parser')
const path = require('path')
const app = express()
const wsServer = require('express-ws')(app)

app.use(bodyParser.urlencoded({extended:true}))
app.use(express.static('public'))

const port = process.env.PORT || 8000

app.get('/scope', (req, res) => {
    res.sendFile(path.join(__dirname, '/public/index.html'))
})

app.post('/sync', (req, res) => {
    res.send(Date.now().toString())
})

app.post('/test', (req, res) => {
    console.log('post request received')
    console.log(req.body)
    res.sendStatus(200)
    sockets.forEach(s => {
        s.send(JSON.stringify(req.body))
    })
})

app.listen(port, () => {
    console.log('app running at port: ' + port)
})

let sockets = []

function handleWs(ws){
    sockets.push(ws)
    ws.on('close', () => {
        sockets.splice(sockets.findIndex(s => s == ws))
    })
}

app.ws('/', handleWs)